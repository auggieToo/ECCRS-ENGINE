#include "klm.h"
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ATOM_INVALID ((atomId)0xFFFFFFFFu)
#define RULE_NOT_FOUND ((u32)0xFFFFFFFFu)
#define RANK_UNASSIGNED ((u32)0xFFFFFFFFu)


static i32 
atomTableInit(atomTable *t, u32 initialCap)
{
    if (initialCap == 0) initialCap = 16;
    
	t->names = malloc(initialCap * sizeof(u8 *));
    t->ids   = malloc(initialCap * sizeof(atomId));
    
	if (!t->names || !t->ids) 
	{
        free(t->names); free(t->ids);
        t->names = NULL; t->ids = NULL;
        t->count = t->capacity = 0;
        return -1;
    }

    t->count = 0;
    t->capacity = initialCap;
    return 0;
}

atomId 
atomLookup(const atomTable *t, const char *name)
{
    for (u32 i = 0; i < t->count; i++)
        if (strcmp((const char *)t->names[i], name) == 0)
            return t->ids[i];
    return ATOM_INVALID;
}


static i32 
atomTableGrow(atomTable *t)
{
    u32 newCap = t->capacity ? t->capacity * 2 : 16;
    u8 **nn = realloc(t->names, newCap * sizeof(u8 *));
    
	if (!nn) return -1;
    
	t->names = nn;
    atomId *ni = realloc(t->ids, newCap * sizeof(atomId));
    
	if (!ni) return -1;
    t->ids = ni;
    t->capacity = newCap;
    return 0;
}


atomId 
atomIntern(atomTable *t, const char *name)
{
    atomId found = atomLookup(t, name);
    if (found != ATOM_INVALID) return found;
 
    if (t->count == t->capacity && atomTableGrow(t) != 0)
        return ATOM_INVALID;
 
    size_t len = strlen(name) + 1;
    u8 *copy = malloc(len);
    if (!copy) return ATOM_INVALID;
    memcpy(copy, name, len);
 
    atomId newId = t->count + 1;          /* ids are dense: 0..count-1 */
    t->names[t->count] = copy;
    t->ids[t->count] = newId;
    t->count++;
    return newId;
}

 i32
kbCopy(const knowledgeBase *src, knowledgeBase *dst)
{
	if (kbInit(dst, src->count ? src->count : 4) != 0) return -1;
 
	for (u32 i = 0; i < src->count; i++)
		if (kbAddRule(dst, src->rules[i].impl.type,
					  &src->rules[i].impl.head,
					  &src->rules[i].impl.body) == RULE_NOT_FOUND)
		{
			kbFree(dst);
			return -1;
		}
	return 0;
}


static void 
atomTableFree(atomTable *t)
{
    if (!t) return;
    for (u32 i = 0; i < t->count; i++)
        free(t->names[i]);
    free(t->names);
    free(t->ids);
    t->names = NULL;
    t->ids = NULL;
    t->count = t->capacity = 0;
}

void 
formulaInit(formula *f)
{
    f->clause = NULL;
    f->count = 0;
}
 
void 
formulaFree(formula *f)
{
    if (!f) return;
    free(f->clause);
    f->clause = NULL;
    f->count = 0;
}

inline literal 
literalNegate(literal l)
{
	l.sign = (l.sign == POSITIVE) ? NEGATIVE : POSITIVE;
	return l;
}


i32 
literalEq(literal a , literal b)
{	
	return	   a.atom == b.atom 
		    && a.sign == b.sign; 

}

i32 
formulaAddLiteral(formula *f, literal l)
{
	//avoid dups 
    for (u32 i = 0; i < f->count; i++)
        if (literalEq(f->clause[i], l))
            return 0;
 
    literal *nc = realloc(f->clause, (f->count + 1) * sizeof(literal));
    if (!nc) return -1;
    f->clause = nc;
    f->clause[f->count++] = l;
    return 0;
}

qimplic
implicFromLits(const knowledgeBase *kb, ruleType t,u32 id, litList body, litList head)
{
    qimplic r = { .type = t , .queryId = id};
    assert(body.n <= MAX_CLAUSE);
    r.body.count = body.n;
    for (u32 i = 0; i < body.n; i++) {
        atomId a = atomIntern(&kb->atoms, body.lits[i].name);
        r.body.clause[i] = (literal){a, body.lits[i].t};
    }
    assert(head.n <= MAX_CLAUSE);
    r.head.count = head.n;
    for (u32 i = 0; i < head.n; i++) {
        atomId a = atomIntern(&kb->atoms, head.lits[i].name);
        r.head.clause[i] = (literal){a, head.lits[i].t};
    }
    return r;
}


i32 
formulaCopy(formula *dst, const formula *src)
{
    dst->count = src->count;

    if (src->count == 0) 
	{
			dst->clause = NULL; 
			return 0; 
	}
    dst->clause = malloc(src->count * sizeof(literal));
    
	if (!dst->clause) 
	{ 
		dst->count = 0; return -1; 
	}
	
    memcpy(dst->clause, src->clause, src->count * sizeof(literal));
    return 0;
}

i32 
formulaContains(const formula *f, literal l)
{
    for (u32 i = 0; i < f->count; i++)
        if (literalEq(f->clause[i], l))
            return 1;
    return 0;
}

i32 formulaEq(const formula *a, const formula *b)
{
    if (a->count != b->count) return 0;
    for (u32 i = 0; i < a->count; i++)
        if (!formulaContains(b, a->clause[i]))
            return 0;
    return 1;
}




static void 
implicFree(implic *im)
{
    formulaFree(&im->head);
    formulaFree(&im->body);
}
 
static i32 
implicCopy(implic *dst, const implic *src)
{
    dst->type = src->type;
    if (formulaCopy(&dst->head, &src->head) != 0) return -1;
    
	if (formulaCopy(&dst->body, &src->body) != 0) 
	{
        formulaFree(&dst->head);
        return -1;
    }
    return 0;
}

static inline  i32 
ruleEq(const rule *a, const rule *b)
{
	return a->impl.type == b->impl.type &&
			formulaEq(&a->impl.head, &b->impl.head) &&	
			formulaEq(&a->impl.body, &b->impl.body); 
			
}

i32 
kbInit(knowledgeBase *kb, u32 cap)
{
	if(cap == 0) cap = 16; 
	kb->rules = malloc(cap * sizeof(rule));
	
	if(!kb->rules) 
	{
		kb->count = kb->capacity = 0 ; 
		return -1; 
	}
	kb->count = 0 ; 
	kb->capacity = cap; 
	
	if(atomTableInit(&kb->atoms, cap) != 0)  //init failed
	{
		free(kb->rules);
		kb->rules = NULL; 
		kb->capacity = 0 ;
		return -1;
	
	}


	return 0; 	
}

void 
kbFree(knowledgeBase *kb)
{
	if(!kb) return; 
	
	for(u32 i = 0 ; i < kb->count; i++)
		implicFree(&kb->rules[i].impl); 
	
	free(kb->rules);
	kb->rules = NULL;
	kb->count = kb->capacity = 0 ; 
	atomTableFree(&kb->atoms);
	
}

static i32 
kbGrow(knowledgeBase *kb)
{
	//grow the knowledgeBase by 1.5 
	u32 newCap = kb->capacity ? kb->capacity + kb->capacity / 2 
				 : 16 ; 
	rule *nr = realloc(kb->rules, newCap * sizeof(rule));
	
	if(!nr) return -1;
	kb->rules = nr;
	kb->capacity = newCap;
	
	return 0;
}

//helper function to keep the rule ID consistent
//If we never remove rules from the knowledge , the ruleID 
//should match the index of the rule in KB.rules array  
static ruleId 
kbNextId(const knowledgeBase *kb)
{
    ruleId maxId = 0;
    for (u32 i = 0; i < kb->count; i++)
        if (kb->rules[i].id >= maxId)
            maxId = kb->rules[i].id + 1;
    return maxId;
}



//add a rule to the knowledge base 
ruleId 
kbAddRule(knowledgeBase *kb, 
		 ruleType type, 
		 const formula *head, 
		 const formula *body)
{
	rule r; 
	r.impl.type = type; 
	r.rank = RANK_UNASSIGNED; 
	
	//check if there is a duplicate
	rule pr;
	pr.impl.type = type; 
	pr.impl.head = *head; 
	pr.impl.body = *body; 

	//check for duplicates 
	for(u32 i = 0 ; i < kb->count; i++)
		if (ruleEq(&kb->rules[i], &pr)) return kb->rules[i].id;
	
	if(kb->count == kb->capacity &&			//kb array full
	   kbGrow(kb) != 0						//attempt to grow  kb  array failed 
	  ) return RULE_NOT_FOUND; 
	
	//add the head
	if (formulaCopy(&r.impl.head, head) != 0)
        return RULE_NOT_FOUND;
   
	//add the body
	if (formulaCopy(&r.impl.body, body) != 0) 
	{
        formulaFree(&r.impl.head);
	    return RULE_NOT_FOUND;
	}

	r.id = kbNextId(kb);
	kb->rules[kb->count++] =r ;
	return r.id;	
}


ruleId 
kbAddRuleWithID(knowledgeBase *kb, 
		 ruleType type, 
		 const formula *head, 
		 const formula *body, 
		 u32 ruleId)
{
	rule r; 
	r.impl.type = type; 
	r.rank = RANK_UNASSIGNED; 
	
	//check if there is a duplicate
	rule pr;
	pr.impl.type = type; 
	pr.impl.head = *head; 
	pr.impl.body = *body; 

	//check for duplicates 
	for(u32 i = 0 ; i < kb->count; i++)
		if (ruleEq(&kb->rules[i], &pr)) return kb->rules[i].id;
	
	if(kb->count == kb->capacity &&			//kb array full
	   kbGrow(kb) != 0						//attempt to grow  kb  array failed 
	  ) return RULE_NOT_FOUND; 
	
	//add the head
	if (formulaCopy(&r.impl.head, head) != 0)
        return RULE_NOT_FOUND;
   
	//add the body
	if (formulaCopy(&r.impl.body, body) != 0) 
	{
        formulaFree(&r.impl.head);
	    return RULE_NOT_FOUND;
	}

	r.id = ruleId;
	kb->rules[kb->count++] =r ;
	return r.id;	
}

ruleId 
kbAddRuleWithName(knowledgeBase *kb, 
						ruleType type, 
						const char **headNames,
						const literalType *hSigns,
						u32 hCount,
						const char **bodyNames,
						const literalType *bSigns,
						u32 bCount
						)
{
	formula h, b; 
	formulaInit(&h);
	formulaInit(&b);
	
	//add the head 
	for(u32 i = 0 ; i < hCount ; i++)
	{	
		atomId a = atomIntern(&kb->atoms, headNames[i]);
		if( (a == ATOM_INVALID) ||			//failed to add the atom
			
			//failed to add the literal to the formula 
		   formulaAddLiteral(&h, (literal){.atom = a , .sign = hSigns[i]}) < 0
		  )	 goto fail; 
	}

	for(u32 i = 0 ; i < bCount ; i++)
	{	
		atomId a = atomIntern(&kb->atoms, bodyNames[i]);
		if( (a == ATOM_INVALID) ||			//failed to add the atom
			
			//failed to add the literal to the formula 
		   formulaAddLiteral(&b, (literal){.atom = a , .sign = bSigns[i]}) < 0
		  )	 goto fail; 
	}
	
	ruleId id =  kbAddRule(kb, type, &h, &b);
	formulaFree(&h);
	formulaFree(&b);
	return id;


	fail:
		formulaFree(&h);
		formulaFree(&b);
		return RULE_NOT_FOUND;	


}


ruleId
kbAddRuleLits(knowledgeBase *kb, 
				   ruleType type,
                   litList head, 
                   litList body 
				   )
{
	const char *hn[head.n]; 
	literalType ht[head.n];
	
    for (u32 i = 0; i < head.n; i++) 
	{ 
		hn[i] =  head.lits[i].name; 
		ht[i] = head.lits[i].t; 
	}
	
    const char *bn[body.n]; 
	literalType bt[body.n];
    for (u32 i = 0; i < body.n; i++) 
	{ 
		bn[i] = body.lits[i].name; 
		bt[i] = body.lits[i].t; 
	}
  
	return kbAddRuleWithName(kb, type, hn, ht, head.n, bn, bt, body.n);

}


u8
kbEquals(knowledgeBase a, knowledgeBase b)
{	
	//if the knowledge base contain unequal amount of rules 
	//then they are not the same knowledge base
	if(a.count != b.count) return 0;
	
}

//knowledge base remove first rule 
u8 
kbRemoveFirst(knowledgeBase *k)
{
	if(k->count == 0) return 1; 
	
	memmove(k->rules , k->rules + 1, (k->count - 1) * sizeof(knowledgeBase));
	k->count--;
	return 0;

}

static const char *
atomName(const atomTable *t, atomId a)
{
	if (!t) return "<?>";

	for (u32 i = 0; i < t->count; i++)
		if (t->ids[i] == a)
			return (const char *)t->names[i];

	return "<?>";
}

/* A conjunction. Empty conjunction is logical truth. */
static void
formulaPrint(FILE *out, const atomTable *t, const formula *f)
{
	if (f->count == 0)
	{
		fputs("T", out);
		return;
	}

	for (u32 i = 0; i < f->count; i++)
	{
		if (i) fputs(" & ", out);
		if (f->clause[i].sign == NEGATIVE) fputc('~', out);
		fputs(atomName(t, f->clause[i].atom), out);
	}
}

void
rulePrint(FILE *out, const atomTable *t, const rule *r)
{
	fprintf(out, "  [%u] ", (u32)r->id);

	formulaPrint(out, t, &r->impl.body);
	fputs(r->impl.type == DEFEASIBLE ? "  |~  " : "  ->  ", out);
	formulaPrint(out, t, &r->impl.head);

	if (r->rank == RANK_UNASSIGNED)
		fputs("      (rank: inf)\n", out);
	else
		fprintf(out, "      (rank: %u)\n", r->rank);
}

void
atomTablePrint(FILE *out, const atomTable *t)
{
	if (!out) out = stdout;
	if (!t) { fputs("atomTable: (null)\n", out); return; }

	fprintf(out, "atoms (%u/%u):", t->count, t->capacity);
	for (u32 i = 0; i < t->count; i++)
		fprintf(out, " %s=%u", (const char *)t->names[i], (u32)t->ids[i]);
	fputc('\n', out);
}

void
kbPrint(FILE *out, const knowledgeBase *kb)
{
	if (!out) out = stdout;
	if (!kb) { fputs("knowledgeBase: (null)\n", out); return; }

	fprintf(out, "knowledgeBase: %u rule%s, %u atom%s\n",
			kb->count, kb->count == 1 ? "" : "s",
			kb->atoms.count, kb->atoms.count == 1 ? "" : "s");

	if (kb->count == 0)
	{
		fputs("  (empty)\n", out);
		return;
	}

	/* classical block first, then defeasible: reads the way a DKB is written */
	fputs(" classical:\n", out);
	u32 shown = 0;
	for (u32 i = 0; i < kb->count; i++)
		if (kb->rules[i].impl.type == CLASSICAL)
		{
			rulePrint(out, &kb->atoms, &kb->rules[i]);
			shown++;
		}
	if (shown == 0) fputs("  (none)\n", out);

	fputs(" defeasible:\n", out);
	shown = 0;
	for (u32 i = 0; i < kb->count; i++)
		if (kb->rules[i].impl.type == DEFEASIBLE)
		{
			rulePrint(out, &kb->atoms, &kb->rules[i]);
			shown++;
		}
	if (shown == 0) fputs("  (none)\n", out);
}

void 
kbPrintWithAtoms(FILE *out, const knowledgeBase *kb, const atomTable *atoms)

{          

		if(!out) out = stdout ; 

		if(!kb) 

		{

			fputs("knowledge base is NULL", out);

			return;

		}

	

		if(kb->count == 0)

		{

			fputs("empty", out);

			return;			

		}

		for(u32 i = 0 ; i < kb->count; i++)

			rulePrint(out, atoms, &kb->rules[i]);
}
