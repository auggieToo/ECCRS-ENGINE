#include <stdio.h>
#include <stdlib.h>
#include "../rules.h"
#include "rulesIterators.h"

#define RULES_EQUAL(a, sizea, b,sizeb) isSubset(a, sizea, b ,sizeb) &&\
                                       isSubset(b, sizeb,a, sizea)


#define MAX_OVERRIDERS 10
typedef struct 
{
	u32 out[3];
	
}toCtx;

typedef struct {
    u32 ruleId[MAX_OVERRIDERS];
    u64 mask[MAX_OVERRIDERS];
    u32 n;
    u32 assignments;
    u64 covered;
} CoverStats;

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
static void printAllChainsToCSV(Overides *outset, u32 size, FILE *out);
static u8  sanityCheck(Rule *ruleset, u32 *out);
static u8  strictGlobalExceptionClosure(Rule *ruleset, u32 *out);
static Rule  totalOverride(Rule *ruleset, CoverStats *out);
static u8 ruleContainsFeature(Rule a, featureIndex fi);
static u8 ruleHasConflictingFeature(Rule a, featureIndex fi, u32  val);
static u8 areCompatible(Rule a, Rule b);
static u8 areCompatible2(Rule a, Rule b);
static inline u8 rulesOppositeLabels(Rule a,Rule b);
static u8 existsUncoveredAssignment(Rule r ,Rule *rules, CoverStats *s);
static u8 searchForAssignment(Instance partialAssignment,featureIndex *freeFeatures, 
			      u32 numFreeFeatures,
			      u32 depth, 
			      Rule r, 
			      Rule *ruleset);
static void enumerateAssignments(Instance pa,
                     featureIndex *freeFeatures, u32 numFree,
                     u32 depth, Rule r, Rule *ruleset,
                     CoverStats *s);


static void reportCover(CoverStats *s, Rule r);
static inline u8 featureKeysEquals(featureIndex a, featureIndex b);

//check if the assumptions made by the Alignment theorem 
//hold for the given ECCRS rules. 
//return 1 if all the all the asusmptions hold 
u8 verifyAssumptions(Rule *rules, FILE *out)
{
    u8 violations = 1;

	if(!out)
	{
		out = stdout;

	}
	fprintf(out, "Assumption Verification\n");
	fprintf(out, "	1. Sanity check: ");
	u32 rOut[2];
    if(sanityCheck(rules,rOut))
        fprintf(out, "Passed\n");
    else 
    {   
        fprintf(out,"Failed\n");																																																		
		fprintf(out,"		Reason: Rule %u and Rule %u have identical rule bodies but opposite labels\n\n", rOut[0], rOut[1]);
        violations = 1;
    }
	rOut[0]=rOut[1]=0;
	fprintf(out, "    2. Strict Global Exception Closure check: ");
    if(strictGlobalExceptionClosure(rules,rOut))
        fprintf(out,"Passed\n");
    else
    {
        fprintf(out,"Failed\n");

        fprintf(out, "		Reason: Rule %d and Rule %d have opposite labels," 
							"and are Compatible but neither is a strict subset of the other\n", 
							rOut[0],rOut[1]);
        violations = 1;

    }


	
	fprintf(out, "	3. No Total Override: ");
	CoverStats toOut = {0};
	Rule r;
	if( (r = totalOverride(rules, &toOut)).numConditions ==0)
	{
		fprintf(out, "Passed\n");
	} 
	else
	{
		fprintf(out, "Reason: \n");
		reportCover(&toOut,r);

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
i8 maximalInclusionSet(Rule *app, u8 size, Rule *outSet, Overides *outRides)
{
    computeOverides(app, outRides, size);
    i8 asize = 0;
    for(u32 k = 0; k < size; k++)
    {
        u8 dominated = 0;
        for(u32 j = 0; j < size; j++)
            if(j != k && app[k].numConditions < app[j].numConditions
                      && isSubsetRule(app[k], app[j]))
                { dominated = 1; break; }
        if(!dominated) outSet[asize++] = app[k];
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

void 
writeExplanationTracesCSV(FILE *fp,
                          u32 instanceId,
                          Rule *applicableRules,
                          Overides *overSets,
                          i8 size,
                          Rule *inclusionSet,
                          i8 includeSize,
                          Prediction prediction)
{
	fprintf(fp, "%u,", instanceId);

	//applicableRules
	fprintf(fp, "\"");
	for(int k = 0 ; k < size ;k++)
	{
		fprintf(fp, "r%d", applicableRules[k].ruleId);
		if(k < size - 1) fprintf(fp," ");
	}
	fprintf(fp, "\",");

	//overrides
	fprintf(fp, "\"");
	printAllChainsToCSV(overSets, size , fp);
	fprintf(fp, "\",");

	//inclusion-maximal 
	//

	fprintf(fp, "\"");
	for(int k = 0 ; k < includeSize ;k++)
	{
		fprintf(fp,"r%d", inclusionSet[k].ruleId);
		if(k < includeSize - 1) fprintf(fp," ");
	}
	fprintf(fp, "\",");

	if (prediction == PRED_1)       fprintf(fp, "1");
	else if (prediction == PRED_0)  fprintf(fp, "0");
	else                            fprintf(fp, "abstain");


	fprintf(fp, "\n");
	
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
sanityCheck(Rule *ruleset,u32 *out)
{
    for(u32 i = 0 ; i < SIZE_OF_RULESET ; i++)
    {
        Condition *a = ruleset[i].conditions;
        u32 sizeA = ruleset[i].numConditions;
        for(u32 j = i + 1 ;  j < SIZE_OF_RULESET ; j++)
        {

            Condition *b = ruleset[j].conditions;
            u32 sizeB = ruleset[j].numConditions;

            if(RULES_EQUAL(a, sizeA,b,sizeB))
            {
                if (ruleset[i].label != ruleset[j].label)
				{
					out[0]=i; out[1]=j;
                    return 0;
				}
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
strictGlobalExceptionClosure(Rule *ruleset, u32 *out)
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

            // equal size → cannot be strict subset 
            if (rk->numConditions == rj->numConditions)
                {

				out[0]=k; out[1]=j;
                //printf("RULE %d and %d are opp labels, Compatible and %d is not strict subset of %d as they are equal\n", k,j,k,j);
                return 0;
                }

            // check only the smaller against larger 
            if (rk->numConditions < rj->numConditions)
            {
                if (!isSubsetRule(*rk, *rj))
                {
                    //printf("RULE %d and %d are opp labels, Compatible and %d is not strict subset of %d\n", k,j,k,j);
                    
					out[0]=k; out[1]=j;
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

static Rule  
totalOverride(Rule *ruleset, CoverStats *out)
{
    u8 allOverridden = 1;
    Rule r;
    RULESET_FOREACH_RULE_SAFE(ruleset, r)
    {
        if (!existsUncoveredAssignment(r, ruleset, out)) return r;
    }
    return (Rule){0};

}

//returns the required value of a feature in a rule
static u8 
featureVal(Rule r, featureIndex feat)
{
	featureIndex fvar;
	u8  vvar ;
	RULE_FOREACH_FEAT_VAL_SAFE(r , fvar , vvar)
	{
		if(featureKeysEquals(fvar, feat)) return vvar;
	}
	return 0;
}


static u8
inAllFeatureArray(featureIndex *arr, u32 len, featureIndex val)
{
    for (u32 i = 0; i < len; i++)
        if (featureKeysEquals(arr[i], val)) return 1;
    return 0;
}


static u8
existsUncoveredAssignment(Rule r, Rule *rules, CoverStats *s)
{

	//build a partial assignment 
	//need to figure out which feature is in what index...
    Instance pa = (Instance){0};
    for (u8 k = 0; k < INSTANCE_SIZE; k++)
        if (ruleContainsFeature(r, ALL_FEATURES[k]))
            pa.conditions[pa.size++] =
                (Condition){ ALL_FEATURES[k], featureVal(r, ALL_FEATURES[k]) };


	//collect all the free features -> these are features that are not in the rule conditions
    featureIndex freeFeatures[INSTANCE_SIZE];
    u32 freeSize = 0;
    Rule other;
    RULESET_FOREACH_RULE_SAFE(rules, other)      
    {
		
		//because we want to check if the rule can be overriden by 
		//rule of opposite label for a given instance/partial assignment , we only need 
		//to select features that appear in the rules of opposite labels 
        if (other.label == r.label) continue;
        featureIndex fvar; u32 vvar;

        RULE_FOREACH_FEAT_VAL_SAFE(other, fvar, vvar)
            if (!ruleContainsFeature(r, fvar) &&
                !inAllFeatureArray(freeFeatures, freeSize, fvar))
                freeFeatures[freeSize++] = fvar;
    }

    if (freeSize > 63) { printf("Rule %u: %u free features, too many\n",
                                r.ruleId, freeSize); return 1; }

    *s = (CoverStats){0};
    enumerateAssignments(pa, freeFeatures, freeSize, 0, r, rules, s);

    u64 full = (1ull << s->assignments) - 1;
    return s->covered != full;
}

// static u8
// alreadyDominated(u32 *partialAssignment, Rule *ruleset, u8 ruleLabel)
// {
// 	Rule other ;
// 	RULESET_FOREACH_RULE_SAFE(ruleset, other)
//     {
//
//         if (other.label == ruleLabel)
//             continue;                           // skip same label
//
//         // check if other_rule fires on ALL extensions of partial_assignment
//         // i.e. every condition of other_rule is already satisfied
//         // in the partial assignment (no condition is UNASSIGNED or conflicting)
//         u8 firesOnAllExtensions = 1;
//         u32 f, v;
//         RULE_FOREACH_FEAT_VAL_SAFE(other, f, v)
//         {
//             if (partialAssignment,f] == UNASSIGNED)
//             {
//                 firesOnAllExtensions = 0;    // this feature not yet fixed
//                 break;
//             }
//             if (partial_assignment[f] != v)
//             {
//                 firesOnAllExtensions = 0;    // condition fails → rule wont fire
//                 break;
//             }
//         }
//
//         if (fires_on_all_extensions)
//             return 1;                           // prune: every extension is dominated
//     }
//     return 0;
// }
//
//

static void 
printInstance(Instance ins)
{
		featureIndex fvar;
		u32 vvar;
		INSTANCE_FOREACH_FEAT_VAL_SAFE(ins, fvar, vvar)
		{
			if(fvar.isPair)  
				printf("a(%d, %d) = %d ", fvar.index1, fvar.index2 , vvar);
			
			else printf("a(%d) = %d ", fvar.index1 , vvar);

		}
		printf("\n");
}

static void 
coverBump(CoverStats *s, u32 id, u32 idx)
{
    for (u32 i = 0; i < s->n; i++)
        if (s->ruleId[i] == id) { s->mask[i] |= 1ull << idx; return; }
    if (s->n < MAX_OVERRIDERS) {
        s->ruleId[s->n] = id;
        s->mask[s->n]   = 1ull << idx;
        s->n++;
    }
}

static void
enumerateAssignments(Instance pa,
                     featureIndex *freeFeatures, u32 numFree,
                     u32 depth, Rule r, Rule *ruleset,
                     CoverStats *s)
{
    if (depth == numFree)
    {
        u32 idx = s->assignments++;     
        u32 hits = 0;
        Rule other;
        RULESET_FOREACH_RULE_SAFE(ruleset, other)
        {
            if (other.ruleId == r.ruleId) continue;
            if (other.label  == r.label)  continue;
            if (isApplicable(other, pa) && isSubsetRule(r, other))
            {
                coverBump(s, other.ruleId, idx);
                hits++;
            }
        }
        if (hits) s->covered |= 1ull << idx;
        //else      printInstance(pa);
        return;
    }

    u32 base = pa.size;
    pa.conditions[base] = (Condition){ freeFeatures[depth], 0 };
    pa.size = base + 1;
    enumerateAssignments(pa, freeFeatures, numFree, depth + 1, r, ruleset, s);

    pa.conditions[base] = (Condition){ freeFeatures[depth], 1 };
    enumerateAssignments(pa, freeFeatures, numFree, depth + 1, r, ruleset, s);
}

static inline u8 
rulesOppositeLabels(Rule a,Rule b)
{
    return a.label != b.label;

}

static inline u8 
featureKeysEquals(featureIndex a, featureIndex b)
{

	if(a.isPair  != b.isPair) return 0; 
	if(a.index1  != b.index1) return 0; 
	if(a.isPair && a.index2  != b.index2) return 0; 
	
	return 1;

}



//check if Condition set a is a subset of Condition set b 
static i8 
isSubset(Condition *a, u32 sizeA,
         Condition *b, u32  sizeB)
{

    u8 match = 0;
    for(int k = 0 ; k < sizeA ; k++ )
    {
        featureIndex aFeatureIdx  = a[k].feature; 
        i32 aRequiredVal = a[k].requiredValue; 

        for(int j = 0 ; j < sizeB ; j++)
            if(featureKeysEquals(aFeatureIdx, b[j].feature) && 
               aRequiredVal == b[j].requiredValue)
                match++;

    }

    return match == sizeA;

}

//check if rule a is a subset of rule b 
//that is : each conditions in rule a must be in rule b 
//return 1 if subset else 0
//Strict
static i8 
isSubsetRule(Rule ar, Rule br)
{
	featureIndex aFidx, bFidx;
    u32 bRval, aRval;
    u32 i, j;

	if (ar.numConditions >= br.numConditions) return 0;

    RULE_FOREACH_FEAT_VAL_IDX(ar, aFidx, aRval, i)
    {
        i8 found = 0;

        RULE_FOREACH_FEAT_VAL_IDX(br, bFidx, bRval, j)
        {
            if (featureKeysEquals(aFidx, bFidx) && 
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
    return a.numConditions < b.numConditions &&
			isSubset(a.conditions, a.numConditions,
                    b.conditions, b.numConditions);

}

//check if a rule contains a feature pattern, (lookup by index)
//return 1 if so else 0;
static u8 
ruleContainsFeature(Rule a, featureIndex fi)
{
	featureIndex f;
    u32 v;
    RULE_FOREACH_FEAT_VAL_SAFE(a,f, v)
    {
        if(featureKeysEquals(f, fi))
            return 1;
    }

    return 0;

}


//return 1 if the feature exists with a different required val;
static u8 
ruleHasConflictingFeature(Rule a,  featureIndex fi, u32  val)
{
	featureIndex f;
    u32 v;
    RULE_FOREACH_FEAT_VAL_SAFE(a,f, v)
    {
        if(featureKeysEquals(f, fi))
             return v != val;
    }
    return 0;

}
 

//check if two rules are Compatible: they can be true together
//return 1 if Compatible else 0
static u8 
areCompatible(Rule a, Rule b)
{
	featureIndex f;
    u32 v;
    RULE_FOREACH_FEAT_VAL_SAFE(a, f, v) //loop over each feature of rule 
    {
        if(ruleHasConflictingFeature(b, f, v))
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
	featureIndex f;
    u32 v;
    RULE_FOREACH_FEAT_VAL_SAFE(a, f, v) //loop over each feature of rule 
    {
        if(ruleHasConflictingFeature(b, f, v))
            return 0;
        if(ruleContainsFeature(b,f)) commonFeature = 1;
        
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


static void
reportCover(CoverStats *s, Rule r)
{
    u64 full = (s->assignments >= 64) ? ~0ull : (1ull << s->assignments) - 1;

    if (s->covered != full) {
        printf("Rule %u: NOT totally overridden (%u of %u completions uncovered)\n",
               r.ruleId, s->assignments - __builtin_popcountll(s->covered),
               s->assignments);
        return;
    }

    u64 need = full;
    printf("Rule %u is totally overridden by:\n", r.ruleId);
    while (need) {
        u32 best = 0; u32 bestGain = 0;
        for (u32 i = 0; i < s->n; i++) {
            u32 gain = __builtin_popcountll(s->mask[i] & need);
            if (gain > bestGain) { bestGain = gain; best = i; }
        }
        if (!bestGain) break;
        printf("  Rule %u (covers %u/%u completions)\n",
               s->ruleId[best], bestGain, s->assignments);
        need &= ~s->mask[best];
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
        printf("Rule %u applies", outset[chain[0]].r.ruleId);
        for (u32 k = 1; k < len; k++)
            printf("Rule %u applies but is more specific than Rule %u, so Rule %u is overridden\n",
                    outset[chain[k]].r.ruleId,
                    outset[chain[k-1]].r.ruleId,
                    outset[chain[k-1]].r.ruleId);
        printf("\n");
    }
}

static void 
printAllChainsToCSV(Overides *outset, u32 size, FILE *out)
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
        fprintf(out,"r%u applies", outset[chain[0]].r.ruleId);
        for (u32 k = 1; k < len; k++)
			{
			 fprintf(out, "->");
             fprintf(out,"r%u applies & overides r%u",
                    outset[chain[k]].r.ruleId,
                    outset[chain[k-1]].r.ruleId);
			}
		if(i < size)
			fprintf(out,"|");
    }
}


