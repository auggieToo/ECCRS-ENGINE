

#include <stdlib.h>
#include <stdio.h>

#include "lex.h"
#include "setRules.h"
#include "orderedTuple.h"



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
	//knowledge base approach 
	kbInit(&ot->R[ot->n].r , 10);

	//set approach 
	rsInit(&ot->R[ot->n].rs, ot->kbSize);


	ot->n++;
	return 0;


}


//add formula to the latest rank 
i8 
tupleAddFormula(orderedTuple *ot, rule r)
{
	return kbAddRule(&ot->R[ot->n-1].r, r.impl.type, &r.impl.head, &r.impl.body);

}

//add the rule to the latest rank 
//idx is the index of the rule in the 
//KB 
void 
tupleAddRule(orderedTuple *ot, u32 idx)
{
	rsAdd(&ot->R[ot->n - 1].rs , idx);
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



u32  unionRank(knowledgeBase *K, orderedTuple ot)
{
	kbInit(K, 10);

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

