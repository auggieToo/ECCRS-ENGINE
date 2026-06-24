//Augustine Mochoeneng , 26-04-2025
//ECCRS Prediction Engine
//NB: THIS IMPLEMENTATION DOES NOT TAKE INTO CONSIDERATION ALIGNMENT Conditions


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eccrsMSW.h"
#include "rules.h"

typedef struct 
{
    u8 checkAssumptions;        //-ca   --disable the Assumptions checks  
    u8 predictionOnly;          //-po   --print the prediction Only
    u8 assumptionsCheckTrace;    //-ct  --print traces for Assumptions checking

}clFlags;

#define DEFUALT_FLAG(flagsName)  \
                 clFlags flagsName = (clFlags){ .predictionOnly   = 0,\
                                       .checkAssumptions = 1,\
                                       .assumptionsCheckTrace = 0\
                                    };\

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
        verifyAssumptions(ruleset);
    }


    for(int k = 0 ; k < SIZE_OF_INSTANCE_SET ; k++)
    {
        // RUN_ECCRS_MSW(ruleset,instanceSet[k].inst, outSet, 
        //                       appliSize, 
        //                       maxInclSize,
        //                       outRides,
        //                       maxInc,
        //                       prediction);

        printf("\n------Instance ID: %d----------\n", instanceSet[k].instanceId);
        printExplanationTraces(outSet, outRides, 
                                       appliSize,
                                       maxInc, 
                                       maxInclSize,
                                       prediction,
                                       flags.predictionOnly);


    }
    return EXIT_SUCCESS;
}

static clFlags 
parseClArguments(i32 argc,char* argv[])
{
    //create a defualt flag
    DEFUALT_FLAG(f);

    for(i32 k = 1 ; k < argc; k++)
    {
        if((strcmp("-ca",argv[k])==0)) f.checkAssumptions = 0;
        else if((strcmp("-po",argv[k])==0)) f.predictionOnly = 1;
        else if((strcmp("-ct",argv[k])==0)) f.assumptionsCheckTrace = 1;
    }

    return f;
}


