//Augustine Mochoeneng , 26-04-2025
//ECCRS Prediction Engine
//NB: THIS IMPLEMENTATION DOES NOT TAKE INTO CONSIDERATION ALIGNMENT Conditions


#include <stdbool.h>
#include <stdlib.h>

#include "eccrsMSW.h" 


i32 main(i32 argc , char * argv[])
{

    u8 appliSize;
    u8 maxInclSize;
    Prediction prediction;

    Rule outSet[MAX_RULES];
    
    Rule maxInc[MAX_RULES];
    Overides outRides[MAX_RULES];


    RUN_ECCRS_MSW(ruleset,F, outSet, appliSize, maxInclSize,outRides,maxInc,prediction);

    printExplanationTraces(outSet, outRides, appliSize, maxInc, maxInclSize,prediction);
    


    return EXIT_SUCCESS;
}


