#pragma once 
#include "klm.h"

typedef struct 
{
	knowledgeBase r; 

}ruleRank; 


typedef struct 
{
	ruleRank *R; 
	u32 n; 

	//resizing 
	u32 _capacity;

	//infinite rank 
	knowledgeBase infinite;
}orderedTuple;

i8 tupleInit(orderedTuple *ot);
i8 tupleNewRank(orderedTuple *ot); 
i8 tupleAddFormula(orderedTuple *ot, rule r);

u32  unionRank(knowledgeBase *K, orderedTuple ot);
void tuplePrint(FILE *out, const atomTable *atoms, const orderedTuple *ot);
