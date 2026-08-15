//author : Augustine Mochoeneng



#include <stdlib.h>
#include <picosat.h>
#include <stdbool.h>


#include "lex.h"
#define TEST_BANK

//sat solver initialization 

//kb size 
u32 kbSize = 0;




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


bool
RationalClosure(knowledgeBase K , implic a)
{
	orderedTuple t = BaseRank(K);

	knowledgeBase R;
	u32 infSize = unionRank(&R,t);
	u32 i = 0;

	while(NegEntail(R, a.body) && R.count - infSize > 0)
	{
		for(int k = 0 ; k < t.R[i].r.count; k++)
		{
			kbRemoveFirst(&R);

		}

		i++;
	}

	return (bool)Entail(R, a);


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

//converts a negated implication to cnf 
//similar to above: R = not (b1 AND b2 AND ... AND bn -> h1 AND h2 ... AND hn) 
static void 
addNegRuleClause(PicoSAT *s, implic *r)
{
	for (u32 i = 0; i < r->body.count; i++) {
        picosat_add(s, litToInt(r->body.clause[i]));
        picosat_add(s, 0);
    }

    for (u32 i = 0; i < r->head.count; i++)
        picosat_add(s, -litToInt(r->head.clause[i]));
    picosat_add(s, 0);

}


static u8 
Entail(knowledgeBase K , implic r)
{

	PicoSAT *solver = picosat_init();
	for(u32 i = 0 ; i < K.count ; i++)
	{	
		addRuleClause(solver, &K.rules[i].impl);
	}

	addNegRuleClause(solver, &r);

	
	int res = picosat_sat(solver,-1);
	
	picosat_reset(solver);
	return res == PICOSAT_UNSATISFIABLE;

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
	tuplePrint(NULL,&A.atoms ,&ot);
	
return 0;

}

/*
LexicographicClosure
1:  Input:  a knowledge base K, a defeasible implication α |~ β
2:  Output: true, if K |≈_LC α |~ β, and false otherwise
3:  (R_0, ..., R_{n-1}, R_inf, n) := BaseRank(K);
4:  i := 0;
5:  R := ⋃_{j=0}^{j<n} R_j;
6:  while R_inf ∪ R |= ¬α  and  R ≠ ∅ do
7:      R := R \ R_i;
8:      W := WeakenRank(R_i, R, R_inf, α);
9:      if W ≠ FAIL then
10:         return R_inf ∪ R ∪ {W} |= α → β;
11:     end if
12:     i := i + 1;
13: end while
14: return R_inf ∪ R |= α → β;

WeakenRank
1:  Input:  a rank R_i, the surviving ranks R, the infinite rank R_inf, an antecedent α
2:  Output: a weakened formula W, or FAIL if no subset of R_i is compatible with α
3:  m := |R_i|;
4:  for k := m - 1 down to 1 do
5:      D_k := ⋁ { ⋀ S  |  S ⊆ R_i,  |S| = k };
6:      if R_inf ∪ R ∪ {D_k} |≠ ¬α then
7:          return D_k;
8:      end if
9:  end for
10: return FAIL;

LexicographicRank                         // rank function form, feeds DefeasibleEntailment
1:  Input:  a knowledge base K
2:  Output: an ordered tuple (L_0, ..., L_{m-1}, L_inf, m)
3:  (R_0, ..., R_{n-1}, R_inf, n) := BaseRank(K);
4:  L_inf := R_inf;
5:  S := ∅;                                // refined ranks, in ascending seriousness
6:  for i := 0 to n - 1 do
7:      m_i := |R_i|;
8:      for k := m_i down to 1 do
9:          D := ⋁ { ⋀ T  |  T ⊆ R_i,  |T| = k };
10:         S := S ⌢ ⟨D⟩;                  // append: larger k = less serious to keep
11:     end for
12: end for
13: (L_0, ..., L_{m-1}) := S;
14: return (L_0, ..., L_{m-1}, L_inf, m);


Seriousness                               // ≺_S, for comparing two subsets directly
1:  Input:  D ⊆ K, base rank function br, order k of K
2:  Output: the tuple n_D = ⟨n_0, ..., n_k⟩
3:  n_0 := |{ α |~ β ∈ D  |  br(α) = ∞ }|;
4:  for i := 1 to k do
5:      n_i := |{ α |~ β ∈ D  |  br(α) = k - i }|;
6:  end for
7:  return ⟨n_0, ..., n_k⟩;

    D_1 ≺_S D_2  iff  n_{D_1} <_lex n_{D_2}      // compared left to right, ∞ first

*/
