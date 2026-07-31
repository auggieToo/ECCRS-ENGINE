#include "klm.h"
#include <stdlib.h>
#include <picosat.h>
#include <stdbool.h>
#define TEST_BANK

//sat solver initialization 

//kb size 
u32 kbSize = 0;

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

//return true if 'concl' is a logical consequence of 'premise'
static u8 logicalConsequence(formula premise, formula concl);


//return true if the implication r is entailed
//by the knowledge base K, 
static u8 NegEntail(knowledgeBase K , formula r);

//materialize the knowledge base 
//turn every defeasible implication into a classical 
//implication;
static void materialisation(knowledgeBase K, knowledgeBase *out);


orderedTuple 
BaseRank(knowledgeBase K)
{
	//initialize our solver

	orderedTuple ot; 
	tupleInit(&ot);

	//algorithm for  BaseRank
	knowledgeBase kArrow = {0};
	kArrow.count	= K.count;
	kArrow.capacity = K.count; 
	materialisation(K, &kArrow);

	
	//E_o = K^(->)
	knowledgeBase E_i;
	kbCopy(&K, &E_i);

	rank:
		
		//init new rank
		tupleNewRank(&ot);

		knowledgeBase E_i1;
		kbInit(&E_i1, 4);

		//tracks whether E_i+1 is the same as E_i 
		u32 inCurrRank  = 0;  	
		
		//E_(i+1) = {a -> B in E_i | E_i entails not a}
		for(int k = 0 ; k < E_i.count ; k++)
		{
			if(	E_i.rules[k].impl.type == CLASSICAL ||			//classical rules live to rank inf
				NegEntail(E_i, E_i.rules[k].impl.body ))
			{
				kbAddRule(&E_i1, 
						  E_i.rules[k].impl.type, 
						  &E_i.rules[k].impl.head ,
						  &E_i.rules[k].impl.body);
	
			}
			else
			{
				//R_i = E_i \ E_(i+1)
				//R_i is all the rules in the E_i that were not added in E_i1 		
				tupleAddFormula(&ot, E_i.rules[k]);
				inCurrRank++;
			}

		}

		if(inCurrRank==0)
		{
			kbFree(&E_i1);
			goto done;
		}

				
		kbFree(&E_i);
		E_i = E_i1;
		goto rank;
		

	done:
		kbInit(&ot.infinite, 8);
		ot.infinite = E_i;
		return ot;
}


u32 
unionRank(knowledgeBase *K, orderedTuple ot)
{
	kbInit(K, kbSize);

	for(u32 j = 0 ; j < ot.n ; j++)
	{
		knowledgeBase r = ot.R[j].r;
		for(u32 i = 0 ; i <  r.count ; i++ )
		{
			implic rule = r.rules[i].impl;  
			kbAddRule(K, rule.type , &rule.head ,&rule.body);
		}


	}

	u32 infiniteRankSize = ot.infinite.count;
	rule* r = ot.infinite.rules; 
	for(u32 i = 0 ; i < infiniteRankSize ; i++ )
	{
			implic rule = r[i].impl;	
			kbAddRule(K, rule.type , &rule.head ,&rule.body);
	}
	
	return infiniteRankSize;
	
	
}


bool
RationalClosure(knowledgeBase K , implic a)
{
	orderedTuple t = BaseRank(K);

	knowledgeBase R;
	unionRank(&R,t);
	


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

static inline i32 
litToInt(literal l){

	return l.sign == POSITIVE ? (i32)l.atom : - (i32)l.atom;
}


//K is entailed by r  <==> K U {not r } UNSAT, 
//K is a set of formulas, treated as conjuction , 
//so checking the satisfiability of K U {not r} is checking one big 
//conjuction
//one formula: a1 AND a2 ... an -> az <==> not a1 OR not a2 OR ... OR az


//This function converts an implication statement to a 
// CNF formula , 
// b1 AND  b2 AND ... AND bn -> h1 AND h2 AND ... hm 
// <==>  ~b1 OR ~b2 OR ... ~bn OR hj   .... j : 1 to m    
//
static void 
addRuleClause(PicoSAT *s, implic *r)
{

	if(r->head.count == 0)
	{
		for(u32 i = 0 ; i < r->body.count ; i++)
		{
			picosat_add(s, -litToInt(r->body.clause[i]));
			
		}	
		picosat_add(s,0);
		return;
	}

	for(u32 i = 0 ; i < r->head.count ; i++)
	{
		for(u32 j = 0 ; j < r->body.count ; j++)
		{
			picosat_add(s, -litToInt(r->body.clause[j]));
	
		}
		picosat_add(s, litToInt(r->head.clause[i]));
		picosat_add(s,0);
	
	}

}

static u8 
NegEntail(knowledgeBase K , formula r)
{

	PicoSAT *solver = picosat_init();
	for(u32 i = 0 ; i < K.count ; i++)
	{	
		addRuleClause(solver, &K.rules[i].impl);
	}

	
	for(u32 j = 0 ; j < r.count ;j++)
	{
		picosat_add(solver,	litToInt(r.clause[j]));			
		picosat_add(solver, 0);
	}

	int res = picosat_sat(solver,-1);
	
	picosat_reset(solver);
	return res == PICOSAT_UNSATISFIABLE;

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
	return kbAddRule(&ot->R[ot->n-1].r, r.impl.type, &r.impl.head, &r.impl.body);

}


void
tuplePrint(FILE *out, const atomTable *atoms, const orderedTuple *ot)
{
	if (!out) out = stdout;
 
	for (u32 i = 0; i < ot->n ; i++)
	{
		fprintf(out, "rank %u:\n", i);
		kbPrintWithAtoms(out, &ot->R[i].r, atoms);
	}
 
	fputs("rank inf:\n", out);
	kbPrintWithAtoms(out, &ot->infinite, atoms);
}

int main()
{ 
	knowledgeBase A; 
	kbInit(&A, 10);

#ifdef  TEST_BOATS
	DEFEASIBLE_RULE(&A,
		IF(POS("boat")),
		THEN(POS("float") ));

	DEFEASIBLE_RULE(&A,
		IF(NEG("boat"), POS("leaky") ),
		THEN(POS("bot") ));

	DEFEASIBLE_RULE(&A,
		IF(NEG("floats")),
		THEN(POS("leaky") ));


	DEFEASIBLE_RULE(&A,
		IF(   POS("boat")   ),
		THEN( POS("floats") ));


	DEFEASIBLE_RULE(&A,
		IF(   POS("wooden") ),
		THEN( POS("floats") ));


	DEFEASIBLE_RULE(&A,
		IF(   POS("anchor") ),
		THEN( NEG("floats") ));


	DEFEASIBLE_RULE(&A,
		IF(   POS("wooden"), POS("anchor") ),
		THEN( NEG("floats") ));
	
	CLASSICAL_RULE(&A,
		IF( POS("flyingDutchman")),
		THEN(POS("boat")));

#endif

#ifdef TEST_BANK

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a15_9"), POS("a16_3"), NEG("a4_1"),
		      NEG("a4_2"),  POS("a4_4"),  NEG("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a15_9"), POS("a16_3"), NEG("a4_1"),
		      NEG("a4_2"),  NEG("a4_4"),  NEG("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a10_2"), NEG("a12_2"), NEG("a14_3"),
		      POS("a16_3"), POS("a4_5"),  NEG("a7_2") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a10_2"), NEG("a12_2"), NEG("a14_3"),
		      POS("a16_3"), POS("a4_5"),  NEG("a7_2") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a12_1"), NEG("a14_3"), POS("a15_1"),
		      NEG("a16_3"), POS("a3_5"),  POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a12_1"), NEG("a14_3"), POS("a15_1"),
		      NEG("a16_3"), NEG("a3_5"),  POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a10_2"), POS("a16_3"), NEG("a4_1"),
		      POS("a4_2"),  NEG("a4_5"),  POS("a9_2") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a12_1"), POS("a15_9"), POS("a16_3"),
		      NEG("a4_1"),  NEG("a4_2"),  NEG("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a12_1"), POS("a15_9"), POS("a16_3"),
		      NEG("a4_1"),  NEG("a4_2"),  NEG("a4_5") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a10_2"), POS("a16_3"), NEG("a4_1"),
		      POS("a4_2"),  NEG("a4_5"),  NEG("a9_2") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a12_1"), NEG("a14_3"), POS("a15_1"),
		      NEG("a16_3"), POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a12_2"), NEG("a14_3"), POS("a16_3"),
		      POS("a4_5"),  POS("a7_2") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a10_2"), POS("a16_3"), NEG("a4_1"),
		      POS("a4_2"),  NEG("a4_5") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a10_2"), POS("a12_2"), NEG("a14_3"),
		      POS("a16_3"), POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a10_2"), POS("a12_2"), NEG("a14_3"),
		      POS("a16_3"), POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a16_3"), NEG("a4_1"), POS("a4_2"), NEG("a4_5") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a14_3"), POS("a16_3"), POS("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a16_3"), POS("a4_1"), NEG("a4_5") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a16_3"), NEG("a4_5") ),
		THEN( POS("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   NEG("a4_5") ),
		THEN( NEG("y") ));

	DEFEASIBLE_RULE(&A,
		IF(   POS("a4_5") ),
		THEN( NEG("y") ));
#endif


	kbPrint(NULL,&A);
	kbSize = A.count;
	

	orderedTuple ot = BaseRank(A);
	tuplePrint(NULL,&A.atoms ,&ot);\
return 0;

}
