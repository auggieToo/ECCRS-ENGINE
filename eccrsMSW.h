#include "rules.h"


typedef struct 
{
    i8 appRulesSize;
    i8 inclusionSetSize;


}RulesetSizes;


//return 1 if 'a; is Compatible to 'b' else 0
i8 isCompatible(Rule a , Rule b);


//return 1 if 'a; is more specific to 'b' else 0
i8 isMoreSpecific(Rule a , Rule b);

//compute the applicable rule given an instance 
static u8 computeApplicableRules(Rule *ruleset, Instance F , Rule* outSet);

//create an Overides list, (rules point to whatever rules overides them)
//static void computeOverides(Rule *applRules,Overides *v, u32 size);


//creates a maximal inclusion set given a applicable sets 
//returns the size of the set 
static i8 maximalInclusionSet(Rule *applicableRules, u8  size, Rule *outSet, Overides *outRides);

//given the inclusion-maximal applicable set, 
//find the prediction
static Prediction mswPrediction(Rule *appRule, i8 size);

//given applicable rules ,overides chain set and inclusion-maximal set, and 
//print the explanation traces.


RulesetSizes 
computeEccrsMSW();

void 
printExplanationTraces(Rule *applicableRules, 
                       Overides  *overSets, 
                       i8 size,
                       Rule *inclusionSet,
                       i8 includeSize,
                       Prediction prediction);

