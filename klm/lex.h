#pragma once 

#include "klm.h"
#include "orderedTuple.h"



//return true if 'concl' is a logical consequence of 'premise'
u8 logicalConsequence(formula premise, formula concl);


//return true if the implication r is entailed
//by the knowledge base K, 
static u8 NegEntail(knowledgeBase K , formula r);
static u8 Entail(knowledgeBase K , implic r);

//materialize the knowledge base 
//turn every defeasible implication into a classical 
//implication;
static void materialisation(knowledgeBase K, knowledgeBase *out);
