#include "rules.h"

#define RUN_ECCRS_MSW(ruleset, F , outset, appliSize, maxInclSize,outRides,maxIncl, prediction )\
        appliSize = computeApplicableRules(ruleset,F,outset);\
        maxInclSize = maximalInclusionSet(outset,appliSize,maxIncl,outRides);\
        prediction = mswPrediction(maxIncl, maxInclSize);\


//return 1 if 'a; is Comparable to 'b' else 0
static inline i8 isComparable(Rule a , Rule b);

//check if Condition set a is a subset of Condition set b 
static i8 
isSubset(Condition *a, u32 sizeA,
         Condition *b, u32  sizeB)
{

    u8 match = 0;
    for(int k = 0 ; k < sizeA ; k++ )
    {
        i32 aFeatureIdx  = a[k].featureIndex; 
        i32 aRequiredVal = a[k].requiredValue; 

        for(int j = 0 ; j < sizeB ; j++)
            if(aFeatureIdx  == b[j].featureIndex && 
               aRequiredVal == b[j].requiredValue)
                match++;

    }

    return match == sizeA;

}


//check if a  is a subset of b
//can be made faster by using hashmap
static inline i8 
isComparable(Rule a, Rule b)
{
    return isSubset(a.conditions, a.numConditions,
                    b.conditions, b.numConditions);

}

//check if a rule  'a' is applicale to an instance F
static inline i8 
isApplicable(Rule a, Instance F)
{
    return isSubset(a.conditions,a.numConditions , 
                    F.conditions, F.size);

}

//compute a set of applicale rules given an Instance F
//returns the size of the set 
u8 
computeApplicableRules(Rule *ruleset, Instance F, Rule *outSet)
{
    u32 size = 0;
    for(int r = 0 ; r < SIZE_OF_RULESET ; r++ )
    {
        if (isApplicable(ruleset[r], F)) outSet[size++] = ruleset[r];

    }

    return size;

}


static int 
compareRuleBySize(const void *a, const void *b)
{
    const Rule *ra = (const Rule *)a;
    const Rule *rb = (const Rule *)b;

    return rb->numConditions - ra->numConditions; 

}


//build a chain of rule overides
//A rule points to the rule which overides it.
static void
computeOverides(Rule *appRules, Overides *outset, u32 size)
{
    qsort(appRules, size, sizeof(Rule), compareRuleBySize);
    for (u32 k = 0; k < size; k++)
        outset[k] = (Overides){ .r = appRules[k],
            .overideRuleId = UINT32_MAX };

    u32 curr      = 0;
    u32 trackRule = 0;
    u32 tagged    = 0;

    while (trackRule < size && tagged < size)
    {
        // Skip tagged rules
        while (trackRule < size && outset[trackRule].overideRuleId != UINT32_MAX)
            trackRule++;
        if (trackRule >= size) break;

        curr = trackRule;
        tagged++;
        u32 nextTrack = size;

        for (u32 x = curr + 1; x < size; x++)
        {
            //skip those already in chain of overides
            if (outset[x].overideRuleId != UINT32_MAX) continue;

            if (isComparable(outset[x].r, outset[curr].r)) //specificity here 
            {
                outset[x].overideRuleId = appRules[curr].ruleId;
                curr = x;
                tagged++;
            }
            else if (nextTrack == size)
            {
                //next root 
                nextTrack = x;
            }
        }

        trackRule = (nextTrack < size) ? nextTrack : trackRule + 1;
    }
}



//calculate all the sets which are not overriden by others 
i8
maximalInclusionSet(Rule *applicableRules,u8 size, 
                    Rule *outSet, Overides *outRides)   
{
    computeOverides(applicableRules, outRides, size);
    i8 asize = 0 ;
    for(u32 k = 0 ; k < size ; k++ )
    {
        if(outRides[k].overideRuleId == UINT32_MAX)
            outSet[asize++] = outRides[k].r;
    }

    return asize; 

}

//make the  msw Prediction 
Prediction 
mswPrediction(Rule  *mf, i8 size)
{

    Prediction prelimPred = PRED_ABSTAIN; 

    for(u32 k = 0 ; k < size ; k++)
    {
        if(k == 0) { prelimPred = mf[k].label; continue;}

        if(prelimPred != mf[k].label) return PRED_ABSTAIN;

    }

    return  prelimPred;

}

//find a rule in an array using its rule id ..
//returns the index of the array
static u32 
findByRuleId(Overides *outset, u32 size, u32 ruleId)
{
    for (u32 i = 0; i < size; i++)
        if (outset[i].r.ruleId == ruleId) return i;
    return UINT32_MAX;
}

//explanation traces: Rules point to the Rule that overrides it
static void 
printAllChains(Overides *outset, u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        // FIND RULE THAT APPLIES FIRST
        u8 isTail = 1;
        for (u32 j = 0; j < size; j++)
            if (outset[j].overideRuleId == outset[i].r.ruleId) { isTail = 0; break; }
        if (!isTail) continue;

        // chanin into a temp array
        u32 chain[size];
        u32 len = 0;
        u32 curr = i;
        while (curr != UINT32_MAX)
        {
            chain[len++] = curr;
            u32 parentRuleId = outset[curr].overideRuleId;
            if (parentRuleId == UINT32_MAX) break;
            curr = findByRuleId(outset, size, parentRuleId);
        }


        //explnation traces
        printf("Rule %u applies\n", outset[chain[0]].r.ruleId);
        for (u32 k = 1; k < len; k++)
            printf("Rule %u applies but is more specific than Rule %u, so Rule %u is overridden\n",
                    outset[chain[k]].r.ruleId,
                    outset[chain[k-1]].r.ruleId,
                    outset[chain[k-1]].r.ruleId);
        printf("\n");
    }
}

void 
printExplanationTraces(Rule *applicableRules, 
                       Overides  *overSets, 
                       i8 size,
                       Rule *inclusionSet,
                       i8 includeSize,
                       Prediction prediction
                       )
{    
    printf("Applicable Rules\n");
    printf("----------------------------\n");
    if(size)
    {
        for(int k = 0 ; k < size ;k++)
            printf("rule %d\n", applicableRules[k].ruleId);
    }
    else printf("\n No Applicable Rules for the given Instance");

    
    printf("\n");
    printf("Overiddes\n");
    printf("----------------------------\n");
    if(size)
        printAllChains(overSets, size);
    else printf("\n No Overiddes\n");

    printf("inclusion-maximal applicable set\n");
    printf("----------------------------\n");
    for(int k = 0 ; k < includeSize ;k++)
    {
        printf("rule %d\n", inclusionSet[k].ruleId);

    }



    printf("\n");
    printf("Prediction\n");
    printf("----------------------------\n");
    if(prediction == PRED_1) printf("Prediction: 1\n"); 
    else if(prediction == PRED_0) printf("Prediction: 0\n"); 
    else printf("Abstain"); 
}
