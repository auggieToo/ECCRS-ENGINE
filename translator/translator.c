
#include <stdio.h>
#include "translator.h"


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
main()
{
	FILE *f = fopen("../kb.c", "w");
	
	
	emitKnowledgeBase(f, ruleset, SIZE_OF_RULESET);
	emitQimplicList(f,instanceSet, 10);

	

}
