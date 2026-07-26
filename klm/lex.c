#include "klm.h"
#include <stdlib.h>
#include <picosat.h>


//sat solver initialization 
PicoSAT *solver;

typedef struct 
{
	knowledgeBase *r; 

}ruleRank; 


typedef struct 
{
	ruleRank *R; 
	u32 n; 

	//resizing 
	u32 _capacity;
}orderedTuple;

i8 tupleInit(orderedTuple *ot);
i8 tupleNewRank(orderedTuple *ot); 
i8 tupleAddFormula(orderedTuple *ot, rule r);

//return true if 'concl' is a logical consequence of 'premise'
static u8 logicalConsequence(formula premise, formula concl);


//return true if the implication r is entailed
//by the knowledge base K, 
static u8 entail(knowledgeBase K , formula r);

//materialize the knowledge base 
//turn every defeasible implication into a classical 
//implication;
static void materialisation(knowledgeBase K, knowledgeBase *out);


orderedTuple 
BaseRank(knowledgeBase K)
{
	//initialize our solver 
	solver = picosat_init();
	orderedTuple ot; 
	tupleInit(&ot);

	//algorithm for  BaseRank
	knowledgeBase kArrow = {0};
	kArrow.count	= K.count;
	kArrow.capacity = K.count; 
	materialisation(K, &kArrow);

	u32 i = 0; 
	
	//E_o = K^(->) 
	knowledgeBase E_i = kArrow; 

	do {
		//an array to keep track of the formula we inserted 
		//in the current rank 
		u32 ranks[E_i.count];
		i32 size = -1;
		
		
		//init new rank
		tupleNewRank(&ot);

		knowledgeBase E_i1;
		kbInit(&E_i1, 4);
		
		//E_(i+1) = {a -> B in E_i | E_i entails not a}
		for(int k = 0 ; k < E_i.count ; k++)
		{
			if(entail(E_i, E_i.rules[k].impl.body ))
			{
				kbAddRule(&E_i1, 
						  E_i.rules[k].impl.type, 
						  &E_i.rules[k].impl.head ,
						  &E_i.rules[k].impl.body);
				ranks[++size] = k;
			}
			
			//R_i = E_i \ E_(i+1)
			//R_i is all the rules in the E_i that were not added in E_i1 		
			tupleAddFormula(&ot, E_i.rules[k]);

		}
}
		

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

i8 
tupleInit(orderedTuple *ot)
{
	
	ot->_capacity = 8;

	ot->R = malloc(ot->_capacity*sizeof(ruleRank));
	ot->n = 0;
		
}

i8
tupleNewRank(orderedTuple *ot)
{	
	if(ot->n == ot->_capacity)
	{
		//grow 
		ot->_capacity = ot->_capacity * 1.5f ;
		ruleRank *nf = realloc(&ot->R, ot->_capacity * sizeof(ruleRank));
		if(!nf) return -1;
		
		ot->R = nf; 
	}
	ot->n++;
	return 0;

}


//add formula to the latest rank 
i8 
tupleAddFormula(orderedTuple *ot, rule r)
{
	kbAddRule(ot->R[ot->n].r, r.impl.type, &r.impl.head, &r.impl.body);
}

