
#ifndef RULE_ITERATORS_H
#define RULE_ITERATORS_H


#define RI_CONCAT_(a,b) a##b
#define RI_CONCAT(a,b)  RI_CONCAT_(a,b)


#define RULESET_FOREACH_RULE_IDX(rulesetp, rulevar, i) \
    for ((i) = 0; \
         (i) < SIZE_OF_RULESET && \
         (((rulevar) = (rulesetp)[(i)]), 1); \
         ++(i))

#define RULESET_FOREACH_RULEPTR_IDX(rulesetp, ruleptr, i) \
    for ((i) = 0; \
         (i) < SIZE_OF_RULESET && \
         (((ruleptr) = &((rulesetp)[(i)])), 1); \
         ++(i))


#define RULESET_FOREACH_RULE_SAFE(rulesetp, rulevar) \
    for (u32 RI_CONCAT(_i_, __LINE__) = 0; \
         RI_CONCAT(_i_, __LINE__) < SIZE_OF_RULESET && \
         (((rulevar) = (rulesetp)[RI_CONCAT(_i_, __LINE__)]), 1); \
         ++RI_CONCAT(_i_, __LINE__))



// Indexed feature/value 
#define RULE_FOREACH_FEAT_VAL_IDX(rulep, fvar, vvar, i) \
    for ((i) = 0; \
         (i) < (rulep).numConditions && \
         (((fvar) = (rulep).conditions[(i)].featureIndex), \
          ((vvar) = (rulep).conditions[(i)].requiredValue), 1); \
         ++(i))

// ndexed condition pointer 
#define RULE_FOREACH_COND_IDX(rulep, condptr, i) \
    for ((i) = 0; \
         (i) < (rulep).numConditions && \
         (((condptr) = &((rulep).conditions[(i)])), 1); \
         ++(i))

// auto-index feature/value 
#define RULE_FOREACH_FEAT_VAL_SAFE(rulep, fvar, vvar) \
    for (u32 RI_CONCAT(_i_, __LINE__) = 0; \
         RI_CONCAT(_i_, __LINE__) < (rulep).numConditions && \
         (((fvar) = (rulep).conditions[RI_CONCAT(_i_, __LINE__)].featureIndex), \
          ((vvar) = (rulep).conditions[RI_CONCAT(_i_, __LINE__)].requiredValue), 1); \
         ++RI_CONCAT(_i_, __LINE__))




// Indexed feature/value 
#define INSTANCE_FOREACH_FEAT_VAL_IDX(instp, fvar, vvar, i) \
    for ((i) = 0; \
         (i) < (instp).size && \
         (((fvar) = (instp).conditions[(i)].featureIndex), \
          ((vvar) = (instp).conditions[(i)].requiredValue), 1); \
         ++(i))

// indexed condition pointer
#define INSTANCE_FOREACH_COND_IDX(instp, condptr, i) \
    for ((i) = 0; \
         (i) < (instp).size && \
         (((condptr) = &((instp).conditions[(i)])), 1); \
         ++(i))

// auto-index feature/value 
#define INSTANCE_FOREACH_FEAT_VAL_SAFE(instp, fvar, vvar) \
    for (u32 RI_CONCAT(_i_, __LINE__) = 0; \
         RI_CONCAT(_i_, __LINE__) < (instp).size && \
         (((fvar) = (instp).conditions[RI_CONCAT(_i_, __LINE__)].featureIndex), \
          ((vvar) = (instp).conditions[RI_CONCAT(_i_, __LINE__)].requiredValue), 1); \
         ++RI_CONCAT(_i_, __LINE__))




#define RULE_FOREACH_IDX(rulep, i) \
    for ((i) = 0; (i) < (rulep).numConditions; ++(i))

#define INSTANCE_FOREACH_IDX(instp, i) \
    for ((i) = 0; (i) < (instp).size; ++(i))

#define RULESET_FOREACH_IDX(i) \
    for ((i) = 0; (i) < SIZE_OF_RULESET; ++(i))




#endif 

