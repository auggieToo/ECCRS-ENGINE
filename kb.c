//---------------------------------------------
//Warning: This file was auto generated
//Changing this file may lead to unexpected behavior
//---------------------------------------------


#include  "klm/klm.h"


void getKnowledgeBase(knowledgeBase *A)
{
	DEFEASIBLE_RULE(A,
		IF( NEG("a15_9"), POS("a16_3"), NEG("a4_1"), 
		    NEG("a4_2"), POS("a4_4"), NEG("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a15_9"), POS("a16_3"), NEG("a4_1"), 
		    NEG("a4_2"), NEG("a4_4"), NEG("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a10_2"), NEG("a12_2"), NEG("a14_3"), 
		    POS("a16_3"), POS("a4_5"), NEG("a7_2") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a10_2"), NEG("a12_2"), NEG("a14_3"), 
		    POS("a16_3"), POS("a4_5"), NEG("a7_2") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a12_1"), NEG("a14_3"), POS("a15_1"), 
		    NEG("a16_3"), POS("a3_5"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a12_1"), NEG("a14_3"), POS("a15_1"), 
		    NEG("a16_3"), NEG("a3_5"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a10_2"), POS("a16_3"), NEG("a4_1"), 
		    POS("a4_2"), NEG("a4_5"), POS("a9_2") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a12_1"), POS("a15_9"), POS("a16_3"), 
		    NEG("a4_1"), NEG("a4_2"), NEG("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a12_1"), POS("a15_9"), POS("a16_3"), 
		    NEG("a4_1"), NEG("a4_2"), NEG("a4_5") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a10_2"), POS("a16_3"), NEG("a4_1"), 
		    POS("a4_2"), NEG("a4_5"), NEG("a9_2") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a12_1"), NEG("a14_3"), POS("a15_1"), 
		    NEG("a16_3"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a12_2"), NEG("a14_3"), POS("a16_3"), 
		    POS("a4_5"), POS("a7_2") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a10_2"), POS("a16_3"), NEG("a4_1"), 
		    POS("a4_2"), NEG("a4_5") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a10_2"), POS("a12_2"), NEG("a14_3"), 
		    POS("a16_3"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a10_2"), POS("a12_2"), NEG("a14_3"), 
		    POS("a16_3"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a16_3"), NEG("a4_1"), POS("a4_2"), 
		    NEG("a4_5") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a14_3"), POS("a16_3"), POS("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a16_3"), POS("a4_1"), NEG("a4_5") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a16_3"), NEG("a4_5") ),
		THEN( POS("m") ));
	DEFEASIBLE_RULE(A,
		IF( NEG("a4_5") ),
		THEN( NEG("m") ));
	DEFEASIBLE_RULE(A,
		IF( POS("a4_5") ),
		THEN( NEG("m") ));
}
