//---------------------------------------------
//Warning: This file was auto generated
//Changing this file may lead to unexpected behavior
//Generated using: tests-rules/bcancer-rules.txt , tests-rules/bcancer-inst-csv.csv 
//---------------------------------------------


#pragma once

#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define MAX_RULES         12
#define MAX_CONDITIONS          4
#define MAX_FEATURES          0

#define INSTANCE_SIZE          8

typedef struct
{
    i32 featureIndex;
    i32 requiredValue;
} Condition;

typedef struct
{
    u32 ruleId;
    Condition conditions[MAX_CONDITIONS];
    u32 numConditions;
    u8 label;
} Rule;

typedef struct
{
    Condition conditions[INSTANCE_SIZE];
    u32 size;
} Instance;

typedef struct
{
   u32 instanceId;
   Instance inst;
}InstanceList;

typedef struct
{
    Rule r;
    u32 overideRuleId;
} Overides;

typedef enum
{
PRED_0 , PRED_1 , PRED_ABSTAIN
}Prediction;

extern Rule ruleset[];
extern u8 ALL_FEATURES[];
extern InstanceList instanceSet[];
extern Instance F;
extern const unsigned  SIZE_OF_RULESET;
extern const unsigned  SIZE_OF_INSTANCE_SET;
