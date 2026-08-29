#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "translator.h"

// ---- translator-local atom table (first-appearance ids) ----


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

// returns id, registering on first sight (first-appearance order)
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

// ---- little-endian fixed-width writers (portable across padding/arch) ----

static void wU32(FILE *f, u32 v) {
    u8 b[4] = { v, v>>8, v>>16, v>>24 };
    fwrite(b, 1, 4, f);
}
static void wU8(FILE *f, u8 v) { fwrite(&v, 1, 1, f); }

// one literal: atom id (u32) + sign (u8)
static void wLiteral(FILE *f, u32 atom, u8 sign) {
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

    // ---- PASS 1: intern all atoms, in the exact order KLM would see them ----
    // KLM builds its table while parsing the KB rules first, then queries.
    // Mirror that order so ids agree even if KLM ever rebuilds from names.

    // head atom "m" appears in KB rules — intern in rule order:
    for (u32 r = 0; r < SIZE_OF_RULESET; r++) {
        Rule *rule = &ruleset[r];
        for (u32 c = 0; c < rule->numConditions; c++)
            featureIntern(&atoms, rule->conditions[c].feature);
        atomIntern(&atoms, "m");   // head
    }
    // queries reference the same features + "m"; interning again is a no-op
    // (already present), but walk them so any query-only atom is caught:
    for (u32 q = 0; q < 106; q++) {
        Instance *inst = &instanceSet[q].inst;
        for (u32 c = 0; c < inst->size; c++)
            featureIntern(&atoms, inst->conditions[c].feature);
        atomIntern(&atoms, "m");
    }

    // ---- SECTION 1: atom table (id is implicit = write order) ----
    wU32(f, atoms.count);
    for (u32 i = 0; i < atoms.count; i++) {
        u32 len = (u32)strlen(atoms.names[i]);
        wU32(f, len);
        fwrite(atoms.names[i], 1, len, f);
    }

    // ---- SECTION 2: KB rules ----
    wU32(f, SIZE_OF_RULESET);
    for (u32 r = 0; r < SIZE_OF_RULESET; r++) {
        Rule *rule = &ruleset[r];

        wU8(f, BLOB_DEFEASIBLE);   // all generated rules are defeasible; adjust if some classical

        // body
        wU32(f, rule->numConditions);
        for (u32 c = 0; c < rule->numConditions; c++) {
            Condition *cond = &rule->conditions[c];
            u32 id = featureIntern(&atoms, cond->feature);   // already present -> returns id
            wLiteral(f, id, cond->requiredValue == 1 ? BLOB_POS : BLOB_NEG);
        }

        // head = single literal "m", sign from label
        wU32(f, 1);
        wLiteral(f, atomIntern(&atoms, "m"),
                    rule->label == 1 ? BLOB_POS : BLOB_NEG);
    }

    // ---- SECTION 3: queries ----
    wU32(f, SIZE_OF_INSTANCE_SET);
    for (u32 q = 0; q < SIZE_OF_INSTANCE_SET; q++) {
        Instance *inst = &instanceSet[q].inst;

        wU32(f, instanceSet[q].instanceId);
        wU8(f, BLOB_DEFEASIBLE);

        // body
        wU32(f, inst->size);
        for (u32 c = 0; c < inst->size; c++) {
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


int
main(int argc, char **argv)
{
    // int useBlob = 0;
    // for (int i = 1; i < argc; i++) {
    //     if (strcmp(argv[i], "--blob")  == 0) useBlob = 1;
    //     if (strcmp(argv[i], "--macro") == 0) useBlob = 0;
    // }

    //if (useBlob) 
	emitBlob();

    return 0;
}
