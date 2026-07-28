#include <stdint.h>



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

#define LITLIST(...) ((Lit[]){ __VA_ARGS__ }), \
                     (sizeof((Lit[]){ __VA_ARGS__ }) / sizeof(Lit))

#define RULE(kb, type, HEAD, BODY) \
    kbAddRuleLits((kb), (type), HEAD, BODY)

#define IF     LITLIST
#define THEN   LITLIST

#define DEFEASIBLE_RULE(kb, head, body)  RULE((kb), DEFEASIBLE, head, body)
#define STRICT_RULE(kb, head, body)      RULE((kb), STRICT, head, body)


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
                   lit *head, 
				   u32 nh,
                   lit *body, 
				   u32 nb);

//check whether two knowledge basea are equal 
u8 kbEquals(knowledgeBase a, knowledgeBase b);

u8 kbEqualsP(knowledgeBase *a, knowledgeBase *b);

u8 LexicographicalClosure(knowledgeBase K);

