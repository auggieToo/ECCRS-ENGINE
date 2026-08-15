#pragma once 
#include "klm.h"
#include "setRules.h"


//TODO: change this so that it only capture the index of the rule from the 
//original knowledge base instead of copying the entire rule. 
//
typedef struct 
{
	knowledgeBase r;

	//rule sets 
	ruleSet rs;


}ruleRank; 


typedef struct 
{
	ruleRank *R; 
	u32 n; 

	//resizing 
	u32 _capacity;

	//length of the knowledge base 
	u32 kbSize;

	//infinite rank 
	knowledgeBase infinite;
}orderedTuple;

i8 tupleInit(orderedTuple *ot);
i8 tupleNewRank(orderedTuple *ot); 
i8 tupleAddFormula(orderedTuple *ot, rule r);

void 
tuplePrintSet(FILE *out,
			 const orderedTuple* ot, 
			 knowledgeBase *K);
void  tupleAddRule(orderedTuple *ot, u32 idx);
u32  unionRank(knowledgeBase *K, orderedTuple ot);
void tuplePrint(FILE *out, const atomTable *atoms, const orderedTuple *ot);
