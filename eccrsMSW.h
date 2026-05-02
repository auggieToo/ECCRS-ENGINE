#include "rules.h"

#define RUN_ECCRS_MSW(ruleset, F , outset, appliSize, maxInclSize,outRides,maxIncl, prediction )\
        appliSize = computeApplicableRules(ruleset,F,outset);\
        maxInclSize = maximalInclusionSet(outset,appliSize,maxIncl,outRides);\
        prediction = mswPrediction(maxIncl, maxInclSize);\


//check if the assumptions made by the Alignment theorem 
//hold for the given ECCRS rules. 
//return 1 if all the all the asusmptions hold 
u8 verifyAssumptions(Rule *rules);


//compute the applicable rule given an instance 
//returns the size of the applicable rules
u8 computeApplicableRules(Rule *ruleset, Instance F , Rule* outSet);


//creates a maximal inclusion set given a applicable sets 
//returns the size of the inclusion-maximal applicable rules set. 
i8 maximalInclusionSet(Rule *applicableRules, u8  size, Rule *outSet, Overides *outRides);

//given the inclusion-maximal applicable set, 
//find the prediction
Prediction mswPrediction(Rule *appRule, i8 size);

//given applicable rules ,overides chain set and inclusion-maximal set, and 
//print the explanation traces.
void 
printExplanationTraces(Rule *applicableRules, 
                       Overides  *overSets, 
                       i8 size,
                       Rule *inclusionSet,
                       i8 includeSize,
                       Prediction prediction,
                       u8 po);

