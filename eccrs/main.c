//Augustine Mochoeneng , 26-04-2025
//ECCRS Prediction Engine
//NB: THIS IMPLEMENTATION DOES NOT TAKE INTO CONSIDERATION ALIGNMENT Conditions


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eccrsMSW.h"
#include "../rules.h"
#include <getopt.h>
#include "report.h"


static void
usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s [what] [where] [options]\n"
        "\n"
        "what to report (default: both)\n"
        "  --po           predictions only\n"
        "  --aco          assumption checks only\n"
        "\n"
        "where to send it (default: stdout; repeatable)\n"
        "  --stdout       write to stdout\n"
        "  --out FILE     write text report to FILE\n"
        "  --csv FILE     write CSV traces to FILE\n"
        "\n"
        "options\n"
        "  --rules FILE   read the ruleset from FILE\n"
        "  --ca           disable assumption checking\n"
        "  --ct           trace assumption checking\n",
        prog);
}

static clFlags
parseClArguments(i32 argc, char *argv[])
{
    DEFAULT_FLAG(f);

    static struct option longOpts[] = {
        { "ca",     no_argument,       0, OPT_CA     },
        { "ct",     no_argument,       0, OPT_CT     },
        { "po",     no_argument,       0, OPT_PO     },
        { "aco",    no_argument,       0, OPT_ACO    },
        { "stdout", no_argument,       0, OPT_STDOUT },
        { "out",    required_argument, 0, OPT_OUT    },
        { "csv",    required_argument, 0, OPT_CSV    },
        { "rules",  required_argument, 0, OPT_RULES  },
        { "help",   no_argument,       0, OPT_HELP   },
        { 0, 0, 0, 0 }
    };

    i32 c;
    while ((c = getopt_long(argc, argv, "", longOpts, NULL)) != -1)
    {
        switch (c)
        {
            case OPT_CA:  f.checkAssumptions      = 0; break;
            case OPT_CT:  f.assumptionsCheckTrace = 1; break;

            case OPT_PO:  f.report = REPORT_PREDICTION; break;
            case OPT_ACO: f.report = REPORT_ASSUMPTION; break;

            case OPT_STDOUT: f.sinks |= SINK_STDOUT; break;
            case OPT_OUT: f.sinks |= SINK_FILE; f.outPath = optarg; break;
            case OPT_CSV: f.sinks |= SINK_CSV;  f.csvPath = optarg; break;

            case OPT_RULES: f.rulesPath = optarg; break;

            case OPT_HELP: usage(argv[0]); exit(EXIT_SUCCESS);
            default:       usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    if (!f.sinks) f.sinks = SINK_STDOUT;

    if (f.report == REPORT_ASSUMPTION && !f.checkAssumptions)
    {
        fprintf(stderr, "%s: --aco and --ca contradict\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (optind < argc)
    {
        fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], argv[optind]);
        exit(EXIT_FAILURE);
    }

    return f;
}

i32 main(i32 argc , char * argv[])
{
    clFlags flags = parseClArguments(argc, argv);

	FILE *txt = NULL, *csv = NULL;
	if (flags.sinks & SINK_FILE)
	{
		txt = fopen(flags.outPath, "w");
		if (!txt) { perror(flags.outPath); return EXIT_FAILURE; }
	}
	if (flags.sinks & SINK_CSV)
	{
		csv = fopen(flags.csvPath, "w");
		if (!csv) { perror(flags.csvPath); return EXIT_FAILURE; }
		fprintf(csv, "instance_id,applicable_rules,overrides,incl_max,prediction\n");
	}



    u8 appliSize;
    u8 maxInclSize;
    Prediction prediction;

    Rule outSet[MAX_RULES];
    
    Rule maxInc[MAX_RULES];
    Overides outRides[MAX_RULES];

    if(flags.report != REPORT_PREDICTION && flags.checkAssumptions)
    {
		verifyAssumptions(ruleset,txt);
    }

	if(flags.report != REPORT_ASSUMPTION)
	{
		for(int k = 0 ; k < SIZE_OF_INSTANCE_SET ; k++)
		{
			RUN_ECCRS_MSW(ruleset,instanceSet[k].inst, outSet, 
								  appliSize, 
								  maxInclSize,
								  outRides,
								  maxInc,
								  prediction);


		printf("\n------Instance ID: %d----------\n", instanceSet[k].instanceId);
		printExplanationTraces(NULL,outSet, outRides, 
											appliSize,
										   maxInc, 
											maxInclSize,
											prediction,
											flags.report);
			
			  writeExplanationTracesCSV(csv,
								  instanceSet[k].instanceId,
								  outSet, outRides, appliSize,
								  maxInc, maxInclSize,
								  prediction);
			

		}
	}
	if(csv)	fclose(csv);
	if(txt) fclose(txt);
    return EXIT_SUCCESS;
}





