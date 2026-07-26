#include "klm.h"
#include <stdlib.h>
#include <picosat.h>


//sat solver initialization 
PicoSAT *solver;

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


//return true if 'concl' is a logical consequence of 'premise'
static u8 logicalConsequence(formula premise, formula concl);


//return true if the implication r is entailed
//by the knowledge base K, 
static u8 Entail(knowledgeBase K , implic r);

//materialize the knowledge base 
//turn every defeasible implication into a classical 
//implication;
static void materialisation(knowledgeBase K, knowledgeBase *out);


orderedTuple 
BaseRank(knowledgeBase K)
{
	//initialize our solver 
	solver = picosat_init();

	//algorithm for  BaseRank
	knowledgeBase kArrow = {0};
	kArrow.count	= K.count;
	kArrow.capacity = K.count; 
	materialisation(K, &kArrow);

	u32 i = 0; 
	knowledgeBase E_i = kArrow; 
	do {
		knowledgeBase E_i1 =  ;
		

	}while ()


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

