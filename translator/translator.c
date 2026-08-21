
#include <stdio.h>
#include "translator.h"


void genConditionName(FILE *out, featureIndex f) {
    if (f.isPair)
        fprintf(out, "\"a%d_%d\"", f.index1, f.index2);
    else
        fprintf(out, "\"a%d\"", f.index1);
}

void emitKnowledgeBase(FILE *out, Rule *ruleset, u32 numRules) {
    fprintf(out, "void getKnowledgeBase(knowledgeBase *A)\n{\n");

    for (u32 r = 0; r < numRules; r++) {
        Rule *rule = &ruleset[r];

        fprintf(out, "\tDEFEASIBLE_RULE(A,\n");
        fprintf(out, "\t\tIF( ");

        for (u32 c = 0; c < rule->numConditions; c++) {
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

int 
main()
{
	FILE *f = fopen("../kb.c", "w");
	
	
	emitKnowledgeBase(f, ruleset, SIZE_OF_RULESET);

	

}
