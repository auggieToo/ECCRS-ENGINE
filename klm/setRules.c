
#include <stdlib.h>
#include <string.h>


#include "setRules.h"

//init rule set 
void 
rsInit(ruleSet * r, u32 numRules)
{
	u32 wrds = (numRules + 63) / 64; 
	
	r->bits = calloc(wrds , sizeof(u64));

	r->capacity = numRules;

	r->count = 0;
}


//add a rule(represented an index in the original data) to the
//if the rule is added in the KB that this set 
//represent, call this function afterwards 
void 
rsAdd(ruleSet *r, u32 idx)
{
	r->bits[idx/64] |= ( 1ULL << (idx%64));

	r->count++;
}

//remove the rule in the set 
//flip the bit
//if the rule is removed in the KB that this set 
//represent, call this function afterwards 
void 
rsRemove(ruleSet *r, u32 idx)
{

	r->bits[idx/64] &= ~( 1ULL << (idx%64));
	
	r->count--;
}


u8 
rsContains(ruleSet *r, u32 idx)
{


	 return (r->bits[idx/64] >> (idx%64)) & 1ULL;
}

//returns the difference of two sets 
void 
rsDifference(ruleSet *dst, ruleSet *a , ruleSet *b)
{
	u32 wrds = (a->capacity + 64 ) / 64 ; 
	for(u32 w  = 0; w < wrds; w++ )
		dst->bits[w] = a->bits[w] & ~(b->bits[w]);
}

	 
// generates all k-element subsets of src via combinatorial enumeration
void 
rsForEachSubsetOfSize(ruleSet *src,
						  u32 capacity,
						  u32 k,
                          void (*fn)(ruleSet *subset, void *ctx), 
						  void *ctx)
{
    u32 indices[capacity];
    u32 n = 0;
    for (u32 i = 0; i < capacity; i++)
        if (rsContains(src, i)) indices[n++] = i;

    if (k > n) return;

    u32 combo[k];
    for (u32 i = 0; i < k; i++) combo[i] = i;

    ruleSet sub;
    rsInit(&sub, capacity);

    for (;;) {
        // build subset from combo
        memset(sub.bits, 0, ((capacity + 63) / 64) * sizeof(u64));
		sub.count = 0;
		for (u32 j = 0; j < k; j++) rsAdd(&sub, indices[combo[j]]);
        fn(&sub, ctx);

        // next combination
        int i = k - 1;
        while (i >= 0 && combo[i] == n - k + i) i--;
        if (i < 0) break;
        combo[i]++;
        for (u32 j = i + 1; j < k; j++) combo[j] = combo[j-1] + 1;
    }
    free(sub.bits);
}


void 
rsForEach(ruleSet *s, 
			   void (*fn) (u32 idx, void *ctx),
				void *ctx)
{
	u32 words = (s->capacity + 63) / 64; 
	for(u32 w = 0 ; w < words ; w++)
	{
		u64 bits = s->bits[w];
		while(bits )
		{
			u32 idx = w * 64 + __builtin_ctzll(bits);
			fn(idx, ctx);
				
			//clear the lowest bit
			bits &= bits -1;
		}


	}
}

void 
rsCopy(ruleSet  *dst, ruleSet *src)
{
	u32 wrds = (src->capacity + 63) / 64 ; 
	
	rsInit(dst, src->capacity);

	memcpy(dst->bits , src->bits, wrds * sizeof(u64));
	
	dst->count = src->count;
}

