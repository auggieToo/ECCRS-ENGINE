#pragma once 

#include "klm.h"

typedef struct 
{
	u64 *bits;

	//number of rules the bitsey can track 
	u32 capacity; 

}ruleSet ;

void rsInit(ruleSet * r, u32 numRules);
void rsAdd(ruleSet *r, u32 idx);
void rsRemove(ruleSet *r, u32 idx);
u8 rsContains(ruleSet *r, u32 idx);
void rsDifference(ruleSet *dst, ruleSet *a , ruleSet *b);
void rsForEachSubsetOfSize(ruleSet *src, u32 capacity, u32 k,
                           void (*fn)(ruleSet *subset, void *ctx), void *ctx);

 


