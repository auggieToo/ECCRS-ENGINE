#include "klm.h"


typedef struct 
{
	rule *r; 
	u32 size; 

	//resizing 
	u32 _capacity; 

}ruleRank; 


typedef struct 
{
	ruleRank *R; 
	u32 n; 

	//resizing 
	u32 _capacity;
}orderedTuple;


orderedTuple 
BaseRank(knowledgeBase K)
{



}


