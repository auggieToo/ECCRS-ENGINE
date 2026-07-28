#include "klm.h"
#include <stdlib.h>
#include <picosat.h>


//sat solver initialization 
PicoSAT *solver;

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


	u32 count_i;
	u32 count_i1;

	//algorithm for  BaseRank
	knowledgeBase kArrow = {0};
	kArrow.count	= K.count;
	kArrow.capacity = K.count; 
	materialisation(K, &kArrow);

	u8 isEmpty = 0;

	u32 i = 0; 
	
	//E_o = K^(->) 
	knowledgeBase E_i = kArrow; 

	u8 changed;
	do {
		//an array to keep track of the formula we inserted 
		//in the current rank 
		u32 ranks[E_i.count];
		i32 size = -1;
		
		
		
		//init new rank
		tupleNewRank(&ot);

		knowledgeBase E_i1;
		kbInit(&E_i1, 4);

		//tracks whether E_i+1 is the same as E_i 

		changed  = 0;  	
		
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
				changed = 1; 
	
			}
			else 				
				//R_i = E_i \ E_(i+1)
				//R_i is all the rules in the E_i that were not added in E_i1 		
				tupleAddFormula(&ot, E_i.rules[k]);

		}

		//counts 
		count_i = E_i.count;
		count_i1 = E_i1.count;
		
		
		//i = i + 1
		i++;

		kbFree(&E_i);
		E_i = E_i1; 
		
	}while (count_i != count_i1);

	//R_inf = := E_i+1 
	//last entry in the tuple is the inf rank 
	u32 n;
	if(count_i==0) n = i - 1 ; 
	else n = i ;

	return ot;
}


void 
materialisation(knowledgeBase K, knowledgeBase *out)
{
	

	out->rules = malloc(sizeof(rule) * K.count );
	for(int i = 0 ; i < K.count ; i++ )
	{
		
		out->rules[i] = K.rules[i]; 
		out->rules[i].impl.type = CLASSICAL;  
	}
}


//K is entailed by r  <==> K U {not r } UNSAT, 
//K is a set of formulas, treated as conjuction , 
//so checking the satisfiability of K U {not r} is checking one big 
//conjuction
//one formula: a1 AND a2 ... an -> az <==> not a1 OR not a2 OR ... OR az
//
static u8 
entail(knowledgeBase K , formula r)
{

	picosat_reset(solver);

	for(u32 i = 0 ; i < K.count ; i++)
	{	
		{  //body
			formula f =  K.rules[i].impl.body;
			for(u32 j = 0 ; j < f.count ;j++)
			{
				picosat_add(solver, f.clause[j].sign == POSITIVE ? 
									f.clause[j].atom :	
									-1 * f.clause[j].atom 
									
									);
			}
		}


		{  //head
			formula f =  K.rules[i].impl.head;
			for(u32 j = 0 ; j < f.count ;j++)
			{
				picosat_add(solver, f.clause[j].sign == POSITIVE ? 
									f.clause[j].atom :	
									-1 * f.clause[j].atom 
									
									);
			}
		}

		picosat_add(solver, 0);

	}

	
	{  //fomula
		formula f =  r;
		for(u32 j = 0 ; j < f.count ;j++)
		{
			picosat_add(solver, f.clause[j].sign == POSITIVE ? 
								f.clause[j].atom :	
								-1 * f.clause[j].atom 
								
								);
		}
	}
	picosat_add(solver, 0);

	int res = picosat_sat(solver,-1);
	return 0;

}


i8 
tupleInit(orderedTuple *ot)
{
	
	ot->_capacity = 8;

	ot->R = malloc(ot->_capacity*sizeof(ruleRank));
	ot->n = 0;

	return 1;
		
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
	kbInit(&ot->R[ot->n].r , 10);
	ot->n++;
	return 0;

}


//add formula to the latest rank 
i8 
tupleAddFormula(orderedTuple *ot, rule r)
{
	return kbAddRule(&ot->R[ot->n].r, r.impl.type, &r.impl.head, &r.impl.body);

}



int main()
{ 
	knowledgeBase A; 
	kbInit(&A, 10);
	solver = picosat_init();
	
	DEFEASIBLE_RULE(&A,
		IF(POS("boat")),
        THEN(POS("float") ));
	
	DEFEASIBLE_RULE(&A,
		IF(NEG("boat"), POS("leaky") ),
        THEN(POS("bot") ));

	DEFEASIBLE_RULE(&A,
		IF(NEG("floats")),
        THEN(POS("leaky") ));
	
	printf("hereo: %d", A.count);
	kbPrint(NULL,&A);

	
	return 0;

}
