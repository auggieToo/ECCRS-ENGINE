#include <stdio.h>
#include <stdlib.h>
#include "rules.h"
#include "rulesIterators.h"

#define RULES_EQUAL(a, sizea, b,sizeb) isSubset(a, sizea, b ,sizeb) &&\
                                       isSubset(b, sizeb,a, sizea)



//helper fuctions
static inline i8 isComparable(Rule a , Rule b);
static i8 isSubset(Condition *a, u32 sizeA,Condition *b, u32  sizeB);
static i8 isSubsetRule(Rule a, Rule b);
static u8 isCompatible(Rule a, Rule b);
static inline i8 isApplicable(Rule a, Instance F);
static i32 compareRuleBySize(const void *a, const void *b);
static void computeOverides(Rule *appRules, Overides *outset, u32 size);
static u32  findByRuleId(Overides *outset, u32 size, u32 ruleId);
static void printAllChains(Overides *outset, u32 size);
static u8  sanityCheck(Rule *ruleset);
static u8  strictGlobalExceptionClosure(Rule *ruleset);
static u8  totalOverride(Rule *ruleset);
static u8 ruleContainsFeature(Rule a, u32 featureIndex);
static u8 ruleHasConflictingFeature(Rule a, u32 featureIndex, u32  val);
static u8 areCompatible(Rule a, Rule b);
static u8 areCompatible2(Rule a, Rule b);
static inline u8 rulesOppositeLabels(Rule a,Rule b);
static u8 existsUncoveredAssignment(Rule r ,Rule *rules);
static u8 searchForAssignment(u32 *partialAssignment,u32 *freeFeatures, 
			      u32 numFreeFeatures,
			      u32 depth, 
			      Rule r, 
			      Rule *ruleset);


//check if the assumptions made by the Alignment theorem 
//hold for the given ECCRS rules. 
//return 1 if all the all the asusmptions hold 
u8 verifyAssumptions(Rule *rules)
{
    u8 violations = 1;
    if(sanityCheck(rules))
        printf("Passed Sanity check\n");
    else 
    {   
        printf("Failed Sanity check\n");
        violations = 1;
    }

    if(strictGlobalExceptionClosure(rules))
        printf("Strict Global Exception  Closure not violated\n");
    else
    {
        printf("Strict Global Closure Violated\n");
        violations = 1;

    }

    return violations;

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

//creates a maximal inclusion set given a applicable sets 
//returns the size of the inclusion-maximal applicable rules set. 
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

//given the inclusion-maximal applicable set, 
//find the prediction
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


//given applicable rules ,overides chain set and inclusion-maximal set, and 
//print the explanation traces.
void 
printExplanationTraces(Rule *applicableRules, 
                       Overides  *overSets, 
                       i8 size,
                       Rule *inclusionSet,
                       i8 includeSize,
                       Prediction prediction,
                       u8 po)
{
    if(!po)
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

    }

    printf("\n");
    printf("Prediction\n");
    printf("----------------------------\n");
    if(prediction == PRED_1) printf("Prediction: 1\n"); 
    else if(prediction == PRED_0) printf("Prediction: 0\n"); 
    else printf("Abstain\n"); 
}


//check if there exists two rules with identical bodies but different labels 
//return 0 if such pair of rules exists else 1;
static u8  
sanityCheck(Rule *ruleset)
{
    for(int i = 0 ; i < SIZE_OF_RULESET ; i++)
    {
        Condition *a = ruleset[i].conditions;
        u32 sizeA = ruleset[i].numConditions;
        for(int j = i + 1 ;  j < SIZE_OF_RULESET ; j++)
        {

            Condition *b = ruleset[j].conditions;
            u32 sizeB = ruleset[j].numConditions;

            if(RULES_EQUAL(a, sizeA,b,sizeB))
            {
                if (ruleset[i].label != ruleset[j].label) 
                    return 0;

            }

        }

    }

    return 1;
}



//check for exception closure violation - 
//i.e Two Compatible rules with opposite labels 
//where either rule body is a subset of the other. 
//return 1 if no violation else 0
static u8  
strictGlobalExceptionClosure(Rule *ruleset)
{
    Rule *rk, *rj;
    u32 k, j;

    RULESET_FOREACH_RULEPTR_IDX(ruleset, rk, k)
    {
        for (j = k + 1; j < SIZE_OF_RULESET; ++j)
        {
            rj = &ruleset[j];

            if (!rulesOppositeLabels(*rk, *rj)) continue;
            if (!areCompatible2(*rk, *rj)) continue;

            /* equal size → cannot be strict subset */
            if (rk->numConditions == rj->numConditions)
                {

                printf("RULE %d and %d are opp labels, Compatible and %d is not strict subset of %d as they are equal\n", k,j,k,j);
                return 0;
                }

            /* check only the smaller against larger */
            if (rk->numConditions < rj->numConditions)
            {
                if (!isSubsetRule(*rk, *rj))
                {
                    printf("RULE %d and %d are opp labels, Compatible and %d is not strict subset of %d\n", k,j,k,j);
                    return 0;
                }
            }
            else
            {
                if (!isSubsetRule(*rj, *rk))
                {

                    printf("RULE %d and %d are opp labels, Compatible and %d is not strict subset of %d\n", k,j,k,j);
                    return 0;
                }
            }
        }
    }

    return 1;
}

//" This can be framed as a small search problem over all possible 
// boolean feature assignments. The goal is to determine whether there 
// exists an assignment that satisfies a given rule without being covered 
// by a stricter rule with the opposite label."

static u8  
totalOverride(Rule *ruleset)
{
    Rule r; 
    RULESET_FOREACH_RULE_SAFE(ruleset, r)
    {


     }
    return 1;

}

static u8 
containsFeature(Rule r, u8 feat)
{
	u8 fvar, vvar ;
	RULE_FOREACH_FEAT_VAL_SAFE(r , fvar , vvar)
	{
		if(fvar==feat) return 1;
	}
	return 0;
}

static u8 
existsUncoveredAssignment(Rule r ,Rule *rules)
{
	//build a partial assignment 
	//need to figure out which feature is in what index...
	u8 partialAssignment[INSTANCE_SIZE];
	for(u8 k = 0; k <  INSTANCE_SIZE;k++)
	{
		if()


	}

	return 0;

}

static u8 searchForAssignment(u32 *partialAssignment,u32 *freeFeatures, 
			      u32 numFreeFeatures,
			      u32 depth, 
			      Rule r, 
			      Rule *ruleset)
{
	

	 return 0;
}
static inline u8 
rulesOppositeLabels(Rule a,Rule b)
{
    return a.label != b.label;

}



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

//check if rule a is a subset of rule b 
//that is : eahc conditions in rule a must be in rule b 
//return 1 if subset else 0
static i8 
isSubsetRule(Rule ar, Rule br)
{
    u32 aFval, aRval;
    u32 bFval, bRval;
    u32 i, j;

    RULE_FOREACH_FEAT_VAL_IDX(ar, aFval, aRval, i)
    {
        i8 found = 0;

        RULE_FOREACH_FEAT_VAL_IDX(br, bFval, bRval, j)
        {
            if (aFval == bFval && 
                aRval == bRval)
            {
                found = 1;
                break;
            }
        }

        if (!found)
            return 0; 
    }

    return 1;
}

 



//check if a  is a subset of b
//can be made faster by using hashmap
static inline i8 
isComparable(Rule a, Rule b)
{
    return isSubset(a.conditions, a.numConditions,
                    b.conditions, b.numConditions);

}

//check if a rule contains a feature pattern, (lookup by index)
//return 1 if so else 0;
static u8 
ruleContainsFeature(Rule a, u32 featureIndex)
{
    u32 f,v;
    RULE_FOREACH_FEAT_VAL_SAFE(a,f, v)
    {
        if(f == featureIndex)
            return 1;
    }

    return 0;

}


//return 1 if the feature exists with a different required val;
static u8 
ruleHasConflictingFeature(Rule a, u32 featureIndex, u32  val)
{
    u32 f,v;
    RULE_FOREACH_FEAT_VAL_SAFE(a,f, v)
    {
        if(f == featureIndex)
             return v != val;
    }
    return 0;

}
 

//check if two rules are Compatible: they can be true together
//return 1 if Compatible else 0
static u8 
areCompatible(Rule a, Rule b)
{
    u32 af,av;
    RULE_FOREACH_FEAT_VAL_SAFE(a, af, av) //loop over each feature of rule 
    {
        if(ruleHasConflictingFeature(b, af, av))
            return 0;
        
    }

    return 1;    
}

//check if two rules are Compatible: they can be true together
//return 1 if Compatible else 0
static u8 
areCompatible2(Rule a, Rule b)
{
    u8 commonFeature = 0;
    u32 af,av;

    RULE_FOREACH_FEAT_VAL_SAFE(a, af, av) //loop over each feature of rule 
    {
        if(ruleHasConflictingFeature(b, af, av))
            return 0;
        if(ruleContainsFeature(b,af)) commonFeature = 1;
        
    }

    return commonFeature;    
}




//check if a rule  'a' is applicable to an instance F
static inline i8 
isApplicable(Rule a, Instance F)
{
    return isSubset(a.conditions,a.numConditions , 
                    F.conditions, F.size);

}



//function to compare rule by the number of conditions they have.
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
    //sort the rules by the number of conditions they have.
    qsort(appRules, size, sizeof(Rule), compareRuleBySize);
    
    //set the override rule id to MAX_INT for all rules 
    for (u32 k = 0; k < size; k++)
        outset[k] = (Overides){ .r = appRules[k],
				.overideRuleId = UINT32_MAX 
			      };
	
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
        u32 nextTrack = size;		//the first rule not part of any overide chain

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





//find a rule in an array using its rule id ..
//returns the index of the array
static  inline u32 
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


