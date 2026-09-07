#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "translator.h"



#define MAX_ATOMS 4096
#define MAX_NAME  32

typedef struct {
    char names[MAX_ATOMS][MAX_NAME];
    u32  count;
} transAtoms;

static void
featureName(featureIndex f, char *buf, size_t n)
{
    if (f.isPair) snprintf(buf, n, "a%d_%d", f.index1, f.index2);
    else          snprintf(buf, n, "a%d", f.index1);
}

static u32
atomIntern(transAtoms *t, const char *name)
{
    for (u32 i = 0; i < t->count; i++)
        if (strcmp(t->names[i], name) == 0) return i;

    if (t->count >= MAX_ATOMS) { fprintf(stderr, "atom table full\n"); exit(1); }
    strncpy(t->names[t->count], name, MAX_NAME - 1);
    t->names[t->count][MAX_NAME - 1] = 0;
    return t->count++;
}

static u32
featureIntern(transAtoms *t, featureIndex f)
{
    char name[MAX_NAME];
    featureName(f, name, sizeof name);
    return atomIntern(t, name);
}


//little endian writer
static void 
wU32(FILE *f, u32 v) 
{
    u8 b[4] = { v, v>>8, v>>16, v>>24 };
    fwrite(b, 1, 4, f);
}
static void wU8(FILE *f, u8 v) { fwrite(&v, 1, 1, f); }

static void 
wLiteral(FILE *f, u32 atom, u8 sign) 
{
    wU32(f, atom);
    wU8(f, sign);
}

#define BLOB_NEG 0    // NEGATIVE
#define BLOB_POS 1    // POSITIVE
#define BLOB_DEFEASIBLE 0
#define BLOB_CLASSICAL  1

static void
emitBlob(void)
{
    FILE *f = fopen("../mushroom-data.blob", "wb");
    if (!f) { perror("fopen ../mushroom.blob"); return; }

    transAtoms atoms = { .count = 0 };

	//insert all atoms , in the exact order as seen by the klm, 
	//trying to emulate how atoms are assigned ID. 

    for (u32 r = 0; r < SIZE_OF_RULESET; r++)
	{
        Rule *rule = &ruleset[r];
        for (u32 c = 0; c < rule->numConditions; c++)
            featureIntern(&atoms, rule->conditions[c].feature);
        atomIntern(&atoms, "m");   // head
    }

    for (u32 q = 0; q < SIZE_OF_INSTANCE_SET; q++) 
	{
        Instance *inst = &instanceSet[q].inst;
        for (u32 c = 0; c < inst->size; c++)
            featureIntern(&atoms, inst->conditions[c].feature);
        atomIntern(&atoms, "m");
    }

    //atom table
    wU32(f, atoms.count);
    for (u32 i = 0; i < atoms.count; i++)
	{
        u32 len = (u32)strlen(atoms.names[i]);
        wU32(f, len);
        fwrite(atoms.names[i], 1, len, f);
    }

    //KB rules
    wU32(f, SIZE_OF_RULESET);
    for (u32 r = 0; r < SIZE_OF_RULESET; r++) 
	{
        Rule *rule = &ruleset[r];

		// all generated rules are defeasible
        wU8(f, BLOB_DEFEASIBLE);   
        // body
        wU32(f, rule->numConditions);
        for (u32 c = 0; c < rule->numConditions; c++) 
		{
            Condition *cond = &rule->conditions[c];
            u32 id = featureIntern(&atoms, cond->feature);
            wLiteral(f, id, cond->requiredValue == 1 ? 
							BLOB_POS : BLOB_NEG);
        }

        // head = single literal "m", sign from label
        wU32(f, 1);
        wLiteral(f, atomIntern(&atoms, "m"),
                    rule->label == 1 ? BLOB_POS : BLOB_NEG);
    }

    //queries
    wU32(f, SIZE_OF_INSTANCE_SET);
    for (u32 q = 0; q < SIZE_OF_INSTANCE_SET; q++) 
	{
        Instance *inst = &instanceSet[q].inst;

        wU32(f, instanceSet[q].instanceId);
        wU8(f, BLOB_DEFEASIBLE);

        // body
        wU32(f, inst->size);
        for (u32 c = 0; c < inst->size; c++) 
		{
            Condition *cond = &inst->conditions[c];
            u32 id = featureIntern(&atoms, cond->feature);
            wLiteral(f, id, cond->requiredValue == 1 ? BLOB_POS : BLOB_NEG);
        }

        // head = POS("m")
        wU32(f, 1);
        wLiteral(f, atomIntern(&atoms, "m"), BLOB_POS);
    }

    fclose(f);
}

void 
genConditionName(FILE *out, featureIndex f)
{
    if (f.isPair)
        fprintf(out, "\"a%d_%d\"", f.index1, f.index2);
    else
        fprintf(out, "\"a%d\"", f.index1);
}

void 
writeHeader(FILE *out)
{
	fprintf(out,"//---------------------------------------------\n");
    fprintf(out,"//Warning: This file was auto generated\n");
    fprintf(out,"//Changing this file may lead to unexpected behavior\n");
    fprintf(out,"//---------------------------------------------\n");
    fprintf(out,"\n\n");

	fprintf(out, "#include  \"klm/klm.h\"\n");

	fprintf(out, "\n\n\n");
	
}


void
emitKnowledgeBase(FILE *out, Rule *ruleset, u32 numRules) 
{
	writeHeader(out);
    fprintf(out, "void getKnowledgeBase(knowledgeBase *A)\n{\n");

    for (u32 r = 0; r < numRules; r++) 
{
        Rule *rule = &ruleset[r];

        fprintf(out, "\tDEFEASIBLE_RULE(A,\n");
        fprintf(out, "\t\tIF( ");

        for (u32 c = 0; c < rule->numConditions; c++) 
		{
            Condition *cond = &rule->conditions[c];
            const char *sign = cond->requiredValue == 1 ? "POS" : "NEG";

            fprintf(out, "%s(", sign);
            genConditionName(out, cond->feature);
            fprintf(out, ")");

            if (c + 1 < rule->numConditions) fprintf(out, ", ");
            
			// wrap every 3 conditions for readability
            if ((c + 1) % 3 == 0 && c + 1 < rule->numConditions)
                fprintf(out, "\n\t\t    ");
        }
 
        fprintf(out, " ),\n");

        const char *headSign = rule->label == 1 ? "POS" : "NEG";
        fprintf(out, "\t\tTHEN( %s(\"m\") ));\n", headSign);
    }

    fprintf(out, "}\n");
}

void emitFeatureName(FILE *out, featureIndex f) {
    if (f.isPair)
        fprintf(out, "\"a%d_%d\"", f.index1, f.index2);
    else
        fprintf(out, "\"a%d\"", f.index1);
}

void emitQimplicList(FILE *out, 
					 InstanceList *instances, 
					  u32 numInstances) 
{
	fprintf(out,"\n\n\n\n");
    fprintf(out, "void getQueries(knowledgeBase *A, qimplic *out)\n{\n");

    for (u32 r = 0; r < numInstances; r++) 
	{
        Instance *inst = &instances[r].inst;

        fprintf(out, "\tout[%u] = QUERY(A,%d,\n", r, instances[r].instanceId);
        fprintf(out, "\t\tIF( ");

        for (u32 c = 0; c < inst->size; c++) 
		{
            Condition *cond = &inst->conditions[c];
            const char *sign = cond->requiredValue == 1 ? "POS" : "NEG";

            fprintf(out, "%s(", sign);
            emitFeatureName(out, cond->feature);
            fprintf(out, ")");

            if (c + 1 < inst->size) fprintf(out, ", ");
            if ((c + 1) % 4 == 0 && c + 1 < inst->size)
                fprintf(out, "\n\t\t    ");
        }

        fprintf(out, " ),\n");
        fprintf(out, "\t\tTHEN( POS(\"m\") ));\n\n");
    }

    fprintf(out, "}\n");
}

int
main(int argc, char **argv)
{
    int useBlob = 0;
    for (int i = 1; i < argc; i++) 
	{
         if (strcmp(argv[i], "--blob")  == 0) useBlob = 1;
         if (strcmp(argv[i], "--macro") == 0) useBlob = 0;
     }

    if (useBlob) 
		emitBlob();
	 else 
	{	
		FILE *f = fopen("../kb.c", "w");

		
		emitKnowledgeBase(f, ruleset, SIZE_OF_RULESET);
		emitQimplicList(f,instanceSet, SIZE_OF_INSTANCE_SET);	
	}

    return 0;
}
