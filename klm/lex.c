//author : Augustine Mochoeneng



#include <ctype.h>
#include <stdlib.h>
#include "vendor/picosat.h"
#include <stdbool.h>
#include <string.h>

#include "lex.h"
#include "klm.h"
#include "orderedTuple.h"
#include "setRules.h"
#include "../rules.h"



//sat solver initialization 

//kb size 
u32 kbSize = 0;


void getKnowledgeBase(knowledgeBase *A);
void getQueries(knowledgeBase *A, qimplic *out);


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

	FILE *outf = fopen("../out/baseRank.txt", "w");
	if(!outf){perror("baseRank"); return 0;}

	tuplePrintSet(outf,&B, &K);
	
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
	 fclose(outf);

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
//
/*
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
*/

static void
addRuleClause(PicoSAT *s, implic *r)
{
	for (u32 j = 0; j < r->body.count; j++)
		picosat_add(s, -litToInt(r->body.clause[j]));

	for (u32 i = 0; i < r->head.count; i++)
		picosat_add(s, litToInt(r->head.clause[i]));

	picosat_add(s, 0);
}


/* same, optionally guarded by ¬sel (sel == 0 -> unguarded).
   The guard is added ONCE, not per head literal. */
static void
addRuleClauseSel(PicoSAT *s, const implic *r, i32 sel)
{
	if (sel) picosat_add(s, -sel);

	for (u32 j = 0; j < r->body.count; j++)
		picosat_add(s, -litToInt(r->body.clause[j]));

	for (u32 i = 0; i < r->head.count; i++)
		picosat_add(s, litToInt(r->head.clause[i]));

	picosat_add(s, 0);
}


/* ¬(b1 ∧ ... ∧ bm  →  h1 ∨ ... ∨ hn)
   ≡ b1 ∧ ... ∧ bm ∧ ¬h1 ∧ ... ∧ ¬hn
   Every literal is its OWN unit clause. */
static void
addNegRuleClause(PicoSAT *s, implic *r)
{
	for (u32 i = 0; i < r->body.count; i++) {
		picosat_add(s, litToInt(r->body.clause[i]));
		picosat_add(s, 0);
	}

	for (u32 i = 0; i < r->head.count; i++) {
		picosat_add(s, -litToInt(r->head.clause[i]));
		picosat_add(s, 0);
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
/*
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

}*/


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
/*
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
*/
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

	return nextVar;                 
}

// does  R_start ∧ ... ∧ R_(rankNo-1) ∧ R_inf  entail ¬a ?          
// i.e.  that conjunction ∪ {a}  is UNSAT                           
u8
NegEntailSr(const knowledgeBase *K, const orderedSrTuple *ot,
            u32 startRank, formula a)
{
	PicoSAT *s = picosat_init();
	emitRanks(s, K, ot, startRank);

	for (u32 j = 0; j < a.count; j++) {
		picosat_add(s, litToInt(a.clause[j]));
		picosat_add(s, 0);
	}

	int res = picosat_sat(s, -1);
	picosat_reset(s);
	return res == PICOSAT_UNSATISFIABLE;
}


u8
EntailSr(const knowledgeBase *K, const orderedSrTuple *ot,
         u32 startRank, implic q)
{
	PicoSAT *s = picosat_init();
	emitRanks(s, K, ot, startRank);

	addNegRuleClause(s, &q);                   
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

void loadBlob(const char *path, knowledgeBase *A,
				qimplic **queries, u32 *nQueries);
i32 
main(i32 argc, char *argv[])
{
	u8 blobKB = 1; 
	u8 printBaseRank = 0;
	u8 printSubsetRank  = 0;

	//parse command line arguments 
	for(u32 i = 1 ; i < argc ; i++)
	{
		if(strcmp(argv[i], "--blob") ==0) blobKB = 1;
		if(strcmp(argv[i], "--kbr") ==0) blobKB = 0;
		if(strcmp(argv[i], "--pBRank") ==0) printBaseRank = 1;
		if(strcmp(argv[i], "--pSRank") ==0) printSubsetRank = 1;
		if(strcmp(argv[i], "--out")==0)
		{
			//TODO:  
				
				//everything goes to : outfolder

			i++; 
		}

	}

	knowledgeBase A;
	orderedSrTuple ost;
	srTupleInit(&ost);
	u32 s;
	qimplic *q;

	if(blobKB)
	{
			loadBlob("../data.blob", &A, &q, &s);
	}
	else 
	{
		(void)0;
	}

/*
	knowledgeBase A; 
	kbInit(&A, 10);

	getKnowledgeBase(&A);


	kbPrint(NULL,&A);
	kbSize = A.count;
	

	printf("\n\n\n");

	orderedSrTuple ost;
	srTupleInit(&ost);
	SubsetRankAlg(A, &ost);

	getQueries(&A, q);
*/




	FILE *kbf = fopen("../out/knowledgeBase.txt", "w");
	if(!kbf) {perror("knowledge base"); return 0;}
	kbPrint(kbf,&A);

	kbSize = A.count;
	


	SubsetRankAlg(A, &ost);

	FILE *csv = fopen("../out/lexicogrResults.csv", "w");
	if (!csv) { perror("fopen results.csv"); return 1; }

	fprintf(csv, "instanceid,entail_m,entail_not_m,abstain\n");

	for (u32 i = 0; i < SIZE_OF_INSTANCE_SET; i++) {
		implic qi = asImplic(&q[i]);

		// entail m
		bool entailM = LexicographicClosure(&A, &ost, qi);

		// entail not m
		qi.head.clause[0].sign = NEGATIVE;
		bool entailNotM = LexicographicClosure(&A, &ost, qi);

		// abstain = neither
		int abstain = (!entailM && !entailNotM) ? 1 : 0;

		fprintf(csv, "%u,%d,%d,%d\n",
				q[i].queryId, entailM ? 1 : 0, entailNotM ? 1 : 0, abstain);
	}
fclose(csv);
fclose(kbf);



	
return 0;

}
static u32 rU32(FILE *f) {
    u8 b[4];
    fread(b, 1, 4, f);
    return (u32)b[0] | ((u32)b[1]<<8) | ((u32)b[2]<<16) | ((u32)b[3]<<24);
}
static u8 rU8(FILE *f) { u8 v; fread(&v, 1, 1, f); return v; }


// ids arrive in order 0,1,2,... so this is really append, but we store the
// id explicitly to stay robust if that ever changes.
static void
atomTableAddNamed(atomTable *t, const char *name, atomId id)
{
    if (t->count >= t->capacity) 
	{
        t->capacity = t->capacity ? t->capacity * 2 : 64;
        t->names = realloc(t->names, sizeof(u8*)    * t->capacity);
        t->ids   = realloc(t->ids,   sizeof(atomId) * t->capacity);
    }
    u32 len = strlen(name);
    t->names[t->count] = malloc(len + 1);
    memcpy(t->names[t->count], name, len + 1);
    t->ids[t->count] = id;
    t->count++;
}




// count, then count × (u32 atom, u8 sign)
static void
readFormula(FILE *f, formula *out)   // heap-backed (KB rules)
{
    out->count = rU32(f);
    out->clause = malloc(sizeof(literal) * out->count);
    for (u32 i = 0; i < out->count; i++) {
        out->clause[i].atom = rU32(f) + 1;
        out->clause[i].sign = (literalType)rU8(f);
    }
}

static void
readQformula(FILE *f, qformula *out) // inline (queries)
{
    out->count = rU32(f);
    if (out->count > MAX_CLAUSE) {
        fprintf(stderr, "query clause %u exceeds MAX_CLAUSE %d\n",
                out->count, MAX_CLAUSE);
        exit(1);
    }
    for (u32 i = 0; i < out->count; i++) {
        out->clause[i].atom = rU32(f) + 1;
        out->clause[i].sign = (literalType)rU8(f);
    }
}

void
loadBlob(const char *path, knowledgeBase *A,
         qimplic **queries, u32 *nQueries)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); exit(1); }

    // KB must be initialized (rules array + atom table) before adding
    kbInit(A, 32);

    // SECTION 1: atom table — id = write order
    u32 nAtoms = rU32(f);
    for (u32 i = 0; i < nAtoms; i++) {
        u32 len = rU32(f);
        char name[64];
        fread(name, 1, len, f);
        name[len] = 0;
        atomTableAddNamed(&A->atoms, name, i+1);
    }

	// for (u32 i = 0; i < A->atoms.count; i++)
	//    printf("atom %u = %s (id %u)\n", i, A->atoms.names[i], A->atoms.ids[i]);


    // SECTION 2: rules
    u32 nRules = rU32(f);
    for (u32 r = 0; r < nRules; r++) {
        ruleType type = (ruleType)rU8(f);
        formula body, head;
        readFormula(f, &body);
        readFormula(f, &head);
        kbAddRule(A, type, &head, &body);   
        free(body.clause);          
        free(head.clause);
    }

    // SECTION 3: queries
    *nQueries = rU32(f);
    *queries  = malloc(sizeof(qimplic) * *nQueries);
    for (u32 q = 0; q < *nQueries; q++) {
        qimplic *qi = &(*queries)[q];
        qi->queryId = rU32(f);
        qi->type    = (ruleType)rU8(f);
        readQformula(f, &qi->body);
        readQformula(f, &qi->head);
    }

    fclose(f);
}
