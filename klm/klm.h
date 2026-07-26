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


u8 LexicographicalClosure(knowledgeBase K);

