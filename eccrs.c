//Augustine Mochoeneng , 26-04-2025
//ECCRS Prediction Engine
//NB: THIS IMPLEMENTATION DOES NOT TAKE INTO CONSIDERATION ALIGNMENT Conditions


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eccrsMSW.h"

typedef struct 
{
    u8 checkAssumptions;   //-ca 
    u8 predictionOnly;     //-po

}clFlags;

static clFlags parseClArguments(i32 argc,char* argv[]);




i32 main(i32 argc , char * argv[])
{
    clFlags flags =  parseClArguments(argc,argv);



    u8 appliSize;
    u8 maxInclSize;
    Prediction prediction;

    Rule outSet[MAX_RULES];
    
    Rule maxInc[MAX_RULES];
    Overides outRides[MAX_RULES];

    if(flags.checkAssumptions)
    {
        printf("Assumptions for the give rules holds\n");

    }


    RUN_ECCRS_MSW(ruleset,F, outSet, 
                             appliSize, 
                             maxInclSize,
                             outRides,
                             maxInc,
                             prediction);

    printExplanationTraces(outSet, outRides, 
                                   appliSize,
                                   maxInc, 
                                   maxInclSize,
                                   prediction,
                                   flags.predictionOnly);
    


    return EXIT_SUCCESS;
}

static clFlags 
parseClArguments(i32 argc,char* argv[])
{
    clFlags f = (clFlags){0};

    for(i32 k = 1 ; k < argc; k++)
    {
        if((strcmp("-ca",argv[k])==0)) f.checkAssumptions = 1;
        else if((strcmp("-po",argv[k])==0)) f.predictionOnly = 1;
    }

    return f;
}



