#include "klm.h"
#include <stdlib.h>

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


u8 entails()

//return true if the implication r is entailed
//by the knowledge base K, 
u8 khowledgeBaseEntails(knowledgeBase K , implic r);

//materialize the knowledge base 
//turn every defeasible implication into a classical 
//implication;
void materialisation(knowledgeBase K, knowledgeBase *out);


orderedTuple 
BaseRank(knowledgeBase K)
{
	knowledgeBase kArrow = {0};
	kArrow.count	= K.count;
	kArrow.capacity = K.count; 
	materialisation(K, &kArrow);

	u32 i = 0; 
	E_i = kArrow; 
	do {
		E_i1 

	}while ()
	E


}


void materialisation(knowledgeBase K, knowledgeBase *out)
{
	

	out->rules = malloc(sizeof(rule) * K.count );
	for(int i = 0 ; i < K.count ; i++ )
	{
		
		out->rules[i] = K.rules[i]; 
		out->rules[i].impl.type = CLASSICAL;  
	}
}

