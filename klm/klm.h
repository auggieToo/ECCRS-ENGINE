#include <stdint.h>
#include <stdio.h>


typedef unsigned char u8; 
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32; 
typedef int64_t i64;

 

typedef u32 atomId ; 
typedef u32 ruleId; 


#define POS(a)  ((lit){ (a), POSITIVE })
#define NEG(a)  ((lit){ (a), NEGATIVE })

#define LITLIST(...) \
    ((litList){ (lit[]){ __VA_ARGS__ }, \
                sizeof((lit[]){ __VA_ARGS__ }) / sizeof(lit) })

#define THEN LITLIST
#define IF   LITLIST

#define DEFEASIBLE_RULE(kb, head, body) kbAddRuleLits((kb), DEFEASIBLE, body, head)
#define STRICT_RULE(kb, head, body)     kbAddRuleLits((kb), STRICT, body, head)


typedef enum 
{
	CLASSICAL,
	DEFEASIBLE
}ruleType; 

typedef  enum 
{
	NEGATIVE,
	POSITIVE
}literalType;


typedef struct 
{
	u8 **names;
	atomId *ids;
	u32 count; 
	u32 capacity;

}atomTable; 

typedef struct 
{
	atomId atom; 
	literalType  sign; 
}literal;


typedef struct 
{
	char *name; 
	literalType t; 
}lit; 


typedef struct { lit *lits; u32 n; } litList;
//a conjuctive formula 
typedef  struct 
{
	literal *clause; 
	u32 count;
}formula; 

typedef struct 
{

	//CLASSICAL or DEFEASIBLE
	ruleType  type; 

	//conjuction of literals 
	formula head; 


	//conjuction of literal
	formula body; 
}implic; 

typedef struct 
{
	ruleId id; 

	//
	implic impl; 

	//assigned by RC algorithm 
	u32 rank;
}rule;


typedef struct 
{
	
	rule *rules; 
	u32 count; 
	u32 capacity; 


	atomTable atoms ; 

}knowledgeBase; 


i32 kbInit(knowledgeBase *kb, u32 cap); 
void kbFree(knowledgeBase *kb);

ruleId kbAddRule(knowledgeBase *kb, 
		 ruleType type, 
		 const formula *head, 
		 const formula *body);

ruleId 
kbAddRuleWithName(knowledgeBase *kb, 
						ruleType type, 
						const char **headNames,
						const literalType *hSigns,
						u32 hCount,
						const char **bodyNames,
						const literalType *bSigns,
						u32 bCount
						);

ruleId
kbAddRuleLits(knowledgeBase *kb, 
				   ruleType type,
                   litList head, 
                   litList body 
				   );

//check whether two knowledge basea are equal 
u8 kbEquals(knowledgeBase a, knowledgeBase b);

u8 kbEqualsP(knowledgeBase *a, knowledgeBase *b);

u8 LexicographicalClosure(knowledgeBase K);

void atomTablePrint(FILE *out, const atomTable *t);

void kbPrint(FILE *out, const knowledgeBase *kb);

i32 kbCopy(const knowledgeBase *src, knowledgeBase *dst);
void kbPrintWithAtoms(FILE *out, const knowledgeBase *kb, const atomTable *atoms);

