#pragma  once 
#include "../rules.h"


typedef enum { REPORT_BOTH = 0, REPORT_PREDICTION, REPORT_ASSUMPTION } ReportKind;

enum { SINK_STDOUT = 1u << 0, SINK_FILE = 1u << 1, SINK_CSV = 1u << 2 };

typedef struct
{
    u8          checkAssumptions;      /* --ca    disable assumption checks   */
    u8          assumptionsCheckTrace; /* --ct    trace assumption checking   */
    ReportKind  report;                /* --po / --aco                        */
    u32         sinks;                 /* bitmask of SINK_*                   */
    const char *outPath;               /* --out FILE                          */
    const char *csvPath;               /* --csv FILE                          */
    const char *rulesPath;             /* --rules FILE                        */
} clFlags;

#define DEFAULT_FLAG(name)                          \
    clFlags name = (clFlags){                       \
        .checkAssumptions      = 1,                 \
        .assumptionsCheckTrace = 0,                 \
        .report                = REPORT_BOTH,       \
        .sinks                 = 0,                 \
        .outPath               = NULL,              \
        .csvPath               = "explanation_traces.csv", \
        .rulesPath             = NULL               \
    }
enum { OPT_CA = 1, OPT_CT, OPT_PO, OPT_ACO,
       OPT_STDOUT, OPT_OUT, OPT_CSV, OPT_RULES, OPT_HELP };


