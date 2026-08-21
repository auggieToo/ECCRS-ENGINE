//author : Augustine Mochoeneng



#include <ctype.h>
#include <stdlib.h>
#include <picosat.h>
#include <stdbool.h>


#include "lex.h"
#include "klm.h"
#include "orderedTuple.h"
#include "setRules.h"

#include "../kb.c"

#define TEST_PERSON

//sat solver initialization 

//kb size 
u32 kbSize = 0;




orderedTuple 
BaseRank(knowledgeBase K)
{
	//initialize our solver

	orderedTuple ot; 
	tupleInit(&ot);
	ot.kbSize = K.count;

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
				kbAddRuleWithID(&E_i1, 
						  E_i.rules[k].impl.type, 
						  &E_i.rules[k].impl.head ,
						  &E_i.rules[k].impl.body, 
						  E_i.rules[k].id); 
	
			}
			else
			{
				//R_i = E_i \ E_(i+1)
				//R_i is all the rules in the E_i that were not added in E_i1 		
				tupleAddFormula(&ot, E_i.rules[k]);
				tupleAddRule(&ot,E_i.rules[k].id);
				
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
		
		rsInit(&ot.infRank.rs, E_i.count);
		for(u32 i = 0 ; i < E_i.count ; i++)
		{
			rsAdd(&ot.infRank.rs, E_i.rules[i].id);
		}
		
	
		return ot;
}



void 
fillSrRank(ruleSet *R, void *ctx)
{
	orderedSrTuple *osr = (orderedSrTuple *) ctx; 
	
	subsetRank *sr = &osr->R[osr->rankNo - 1]; 
	rsCopy(&sr->rs[sr->count++] , R);
}


implic
asImplic(qimplic *q)
{
    return (implic){
        .type = q->type,
        .head = { .clause = q->head.clause, .count = q->head.count },
        .body = { .clause = q->body.clause, .count = q->body.count },
    };
}

u32 choose(u32 n, u32 k)
{
	if (k > n) return 0;
	if (k > n - k) k = n - k;
	u64 c = 1;
	for (u32 i = 0; i < k; i++)
		c = c * (n - i) / (i + 1);
	return (u32)c;
}

u32 rsPopcount(ruleSet *s) {
    u32 words = (s->capacity + 63) / 64;
    u32 total = 0;
    for (u32 w = 0; w < words; w++)
        total += __builtin_popcountll(s->bits[w]);
    return total;
}

u32
SubsetRankAlg(knowledgeBase K, orderedSrTuple *out)
{



	orderedTuple B = BaseRank(K);

	tuplePrintSet(NULL,&B, &K);
	
	u32 i = 0, k = 0; 
	
	while(i != B.n)
	{
		 u32 lenBi = rsPopcount(&B.R[i].rs);
		for(u32  j = lenBi ; j >= 1 ; j--)
		{
			srTupleNewRank(out);
			out->R[out->rankNo - 1].count = 0;
			
			u32 cnt = choose(lenBi, j);	
			out->R[out->rankNo - 1].rs = (ruleSet *) malloc(sizeof(ruleSet) * cnt);
			rsForEachSubsetOfSize(&B.R[i].rs, 
								  B.R[i].rs.capacity, 
								  j,
								  fillSrRank, 
								  out);
			k++;
								
		}


		i++;

	}

	out->infRank = B.infRank;

		printf("\n\n\n\n");		
		//orderedSrTuplePrint(NULL, out, &K);
	
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



/* emit one rule's CNF, optionally guarded by ¬sel (sel == 0 -> unguarded) */
static void
addRuleClauseSel(PicoSAT *s, const implic *r, i32 sel)
{
	if (r->head.count == 0)
	{
		if (sel) picosat_add(s, -sel);
		for (u32 i = 0; i < r->body.count; i++)
			picosat_add(s, -litToInt(r->body.clause[i]));
		picosat_add(s, 0);
		return;
	}

	for (u32 j = 0; j < r->head.count; j++)
	{
		if (sel) picosat_add(s, -sel);
		for (u32 i = 0; i < r->body.count; i++)
			picosat_add(s, -litToInt(r->body.clause[i]));
		picosat_add(s, litToInt(r->head.clause[j]));
		picosat_add(s, 0);
	}
}

typedef struct
{
	PicoSAT             *s;
	const knowledgeBase *K;
	i32                  sel;
} emitCtx;

static void
emitRuleCb(u32 idx, void *ctx)
{
	emitCtx *e = (emitCtx *)ctx;
	addRuleClauseSel(e->s, &e->K->rules[idx].impl, e->sel);
}

/* does  R_start ∧ ... ∧ R_(rankNo-1) ∧ R_inf  entail ¬a ?          */
/* i.e.  that conjunction ∪ {a}  is UNSAT                            */
u8
NegEntailSr(const knowledgeBase *K,
            const orderedSrTuple *ot,
            u32 startRank,
            formula a)
{
	PicoSAT *s = picosat_init();
	i32 nextVar = (i32)K->atoms.count + 1;    /* fresh vars live above atoms */

	for (u32 i = startRank; i < ot->rankNo; i++)
	{
		const subsetRank *sr = &ot->R[i];

		if (sr->count == 1)                   /* plain conjunction */
		{
			emitCtx e = { .s = s, .K = K, .sel = 0 };
			rsForEach(&sr->rs[0], emitRuleCb, &e);
		}
		else                                  /* disjunction of conjunctions */
		{
			i32 firstSel = nextVar;

			for (u32 g = 0; g < sr->count; g++)
			{
				emitCtx e = { .s = s, .K = K, .sel = nextVar++ };
				rsForEach(&sr->rs[g], emitRuleCb, &e);
			}

			for (i32 v = firstSel; v < nextVar; v++)   /* sel_1 ∨ ... ∨ sel_p */
				picosat_add(s, v);
			picosat_add(s, 0);
		}
	}

	/* R_inf, unguarded */
	emitCtx e = { .s = s, .K = K, .sel = 0 };
	rsForEach(&ot->infRank.rs, emitRuleCb, &e);

	/* assert a's literals */
	for (u32 j = 0; j < a.count; j++)
	{
		picosat_add(s, litToInt(a.clause[j]));
		picosat_add(s, 0);
	}

	int res = picosat_sat(s, -1);
	picosat_reset(s);
	return res == PICOSAT_UNSATISFIABLE;
}
static i32
emitRanks(PicoSAT *s, const knowledgeBase *K,
          const orderedSrTuple *ot, u32 startRank)
{
	i32 nextVar = (i32)K->atoms.count + 1;

	for (u32 i = startRank; i < ot->rankNo; i++)
	{
		const subsetRank *sr = &ot->R[i];

		if (sr->count == 1)
		{
			emitCtx e = { .s = s, .K = K, .sel = 0 };
			rsForEach(&sr->rs[0], emitRuleCb, &e);
		}
		else
		{
			i32 firstSel = nextVar;
			for (u32 g = 0; g < sr->count; g++)
			{
				emitCtx e = { .s = s, .K = K, .sel = nextVar++ };
				rsForEach(&sr->rs[g], emitRuleCb, &e);
			}
			for (i32 v = firstSel; v < nextVar; v++)
				picosat_add(s, v);
			picosat_add(s, 0);
		}
	}

	emitCtx e = { .s = s, .K = K, .sel = 0 };
	rsForEach(&ot->infRank.rs, emitRuleCb, &e);

	return nextVar;                 /* in case callers need more fresh vars */
}

u8
EntailSr(const knowledgeBase *K, const orderedSrTuple *ot,
         u32 startRank, implic q)
{
	PicoSAT *s = picosat_init();
	emitRanks(s, K, ot, startRank);

	addNegRuleClause(s, &q);                   /* assert ¬(body -> head) */

	int res = picosat_sat(s, -1);
	picosat_reset(s);
	return res == PICOSAT_UNSATISFIABLE;
}

bool
LexicographicClosure(const knowledgeBase *K, const orderedSrTuple *ot, implic q)
{
	u32 i = 0;
	while (i < ot->rankNo && NegEntailSr(K, ot, i, q.body))
		i++;

	return EntailSr(K, ot, i, q);
}

int main()
{ 
	knowledgeBase A; 
	kbInit(&A, 10);

	getKnowledgeBase(&A);


	kbPrint(NULL,&A);
	kbSize = A.count;
	

	printf("\n\n\n");

	orderedSrTuple ost;
	srTupleInit(&ost);
	SubsetRankAlg(A, &ost);

	// qimplic q = QUERY(&A, 0, IF(POS("s")), THEN(POS("m")));
	// implic  qi = asImplic(&q);          
	// bool res = LexicographicClosure(&A, &ost, qi);
	//
	// if(res) printf("Yes"); else printf("No");



	
return 0;

}

