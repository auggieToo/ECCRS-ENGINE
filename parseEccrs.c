//Augustine Mochoeneng - 26-04-202, char *filename6
//ECCRS parser
//Take the raw ECCRS ruleset and produce a C data structure representing
//all the rule
//The rules and the Structs are defined in the generated header and c file ..
//rules.(h / c)

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;



typedef struct 
{
    u32 maxInstances;
    u32 numInstances;

}InstanceSizes;

typedef struct 
{
     u64 pos_max_rules;
     u64 pos_max_conditions;
     u64 pos_max_features;
     u64 pos_max_instance;
     u64 pos_max_instance_list;

} HeaderPatchPoints;

typedef struct 
{
    u32 rulesCount; 
    u32 maxCond;
    u32 maxFeature;
    
} HeaderPatchValues;

typedef struct 
{
    u32 columnIndex

}IndexHeaderLookup;


HeaderPatchPoints writeHeader(FILE *out); 
HeaderPatchValues writeSrc(FILE *out, FILE *in); 

static InstanceSizes writeInstance(FILE *out, FILE *in, char *filename);

i32 main(i32 argc,char *argv[]) 
{
    if(argc < 5)
    {
        printf("Not Given Full information to generate the Rule Data Structure");
        return 1;

    }
    char *ruleName = NULL; 
     char *instanceName = NULL;

    for(i32 k = 1 ; k < argc ; k++)
    {
        if(!(strcmp(argv[k], "-r")))
        {
            if(k + 1 < argc) 
            {   ruleName = argv[++k];
                continue;
            }

        }

        if(!(strcmp(argv[k], "-i")))
        {
            if(k + 1 < argc) 
            {   instanceName = argv[++k];
                continue;
            }

        }

    }

    if(!ruleName)
    {
        printf("Rule file not found");
        return 1;

    }

    if(!instanceName)
    {
        printf("Instance file not found");
        return 1;

    }


    FILE *in = fopen(ruleName, "r");
    if(!in)
    {
        fprintf(stderr,"Could not open the rules file: %s\n", ruleName);
        return 1;

    }
    FILE *inI = fopen(instanceName, "r");
    if(!inI)
    {
        fprintf(stderr,"Could not open the rules file: %s\n", instanceName);
        return 1;

    }


    FILE *out = fopen("rules.h", "w");

    if (!out) 
    {
        perror("file error: Could not the generate the rules.h file");
        return 1;
    }


    FILE *outC = fopen("rules.c", "w");

    if (!outC)
    {
        perror("file error: Could not the generate the rules.c file");
        return 1;
    }

    fprintf(out,"//---------------------------------------------\n");
    fprintf(out,"//Warning: This file was auto generated\n");
    fprintf(out,"//Changing this file may lead to unexpected behavior\n");
    fprintf(out,"//Generated using: %s , %s \n",ruleName, instanceName);
    fprintf(out,"//---------------------------------------------\n");
    fprintf(out,"\n\n");


    fprintf(outC,"//---------------------------------------------\n");
    fprintf(outC,"//Warning: This file was auto generated\n");
    fprintf(outC,"//Changing this file may lead to unexpected behavior\n");
    fprintf(outC,"//Generated using: %s , %s \n",ruleName, instanceName);
    fprintf(outC,"//---------------------------------------------\n\n\n");


    HeaderPatchPoints patch = writeHeader(out);
    HeaderPatchValues pv = writeSrc(outC, in);
    InstanceSizes instances = writeInstance(outC, inI, instanceName);





    fprintf(outC, " const unsigned int SIZE_OF_RULESET = %d;\n", pv.rulesCount);
    fprintf(outC, " const unsigned int SIZE_OF_INSTANCE_SET = %d;\n", instances.numInstances);

    fseek(out, patch.pos_max_rules, SEEK_SET);
    fprintf(out, "%10d", pv.rulesCount);

    fseek(out, patch.pos_max_conditions, SEEK_SET);
    fprintf(out, "%10d", pv.maxCond);

    fseek(out, patch.pos_max_features, SEEK_SET);
    fprintf(out, "%10d", instances.maxInstances);

    fclose(in);
    fclose(out);
    fclose(outC);

    return 0;
}


//write the header file 
HeaderPatchPoints writeHeader(FILE *out) 
{
    HeaderPatchPoints p;

    fprintf(out, "#pragma once\n\n");


    fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "typedef int8_t i8;\n");
    fprintf(out, "typedef int16_t i16;\n");
    fprintf(out, "typedef int32_t i32;\n");
    fprintf(out, "typedef int64_t i64;\n");
    fprintf(out, "typedef uint8_t u8;\n");
    fprintf(out, "typedef uint16_t u16;\n");
    fprintf(out, "typedef uint32_t u32;\n\n");


    fprintf(out, "#define MAX_RULES ");
    p.pos_max_rules = ftell(out);
    fprintf(out, "%10d\n", 0);

    fprintf(out, "#define MAX_CONDITIONS ");
    p.pos_max_conditions = ftell(out);
    fprintf(out, "%10d\n", 0);

    fprintf(out, "#define MAX_FEATURES ");
    p.pos_max_features = ftell(out);
    fprintf(out, "%10d\n\n", 0);


    fprintf(out, "#define INSTANCE_SIZE ");
    p.pos_max_features = ftell(out);
    fprintf(out, "%10d\n\n", 0);

    fprintf(out,
        "typedef struct\n"
        "{\n"
        "    i32 featureIndex;\n"
        "    i32 requiredValue;\n"
        "} Condition;\n\n");

    fprintf(out,
        "typedef struct\n"
        "{\n"
        "    u32 ruleId;\n"
        "    Condition conditions[MAX_CONDITIONS];\n"
        "    u32 numConditions;\n"
        "    u8 label;\n"
        "} Rule;\n\n");

    fprintf(out,
        "typedef struct\n"
        "{\n"
        "    Condition conditions[INSTANCE_SIZE];\n"
        "    u32 size;\n"
        "} Instance;\n\n");

    fprintf(out,
            "typedef struct\n"
            "{\n"
            "   u32 instanceId;\n"
            "   Instance inst;\n"
            "}InstanceList;\n\n");

    fprintf(out,
        "typedef struct\n"
        "{\n"
        "    Rule r;\n"
        "    u32 overideRuleId;\n"
        "} Overides;\n\n");

    fprintf(out, 
            "typedef enum\n"
            "{\n"
            "PRED_0 , PRED_1 , PRED_ABSTAIN\n"
            "}Prediction;\n\n"
            );


    
    fprintf(out, "extern Rule ruleset[];\n");
    fprintf(out, "extern InstanceList instanceSet[];\n");
    fprintf(out, "extern Instance F;\n");
    fprintf(out,"extern const unsigned  SIZE_OF_RULESET;\n");
    fprintf(out,"extern const unsigned  SIZE_OF_INSTANCE_SET;\n");

    return p;
}


//Helper functions to help manually parse the eccrs rules:

//skip spaces in the file
static inline void 
skipSpaces(char **p) 
{
    while (**p == ' ' || **p == '\t') (*p)++;
}

//get an int from a file
static i32 
parseInt(char **p) 
{
    i32 x = 0;
    while (**p >= '0' && **p <= '9') 
    {
        x = x * 10 + (**p - '0');
        (*p)++;
    }
    return x;
}

//get the condtions 
static int 
parseCondition(char **p, int *idx, int *val) {
    skipSpaces(p);

    if (**p != 'a') return 0;
    (*p)++;

    if (**p != '(') return 0;
    (*p)++;

    *idx = parseInt(p);

    if (**p != ')') return 0;
    (*p)++;

    skipSpaces(p);

    if (**p != '=') return 0;
    (*p)++;

    skipSpaces(p);

    *val = parseInt(p);

    return 1;
}

static u8 
endsWith4(const char *src, const char *suf)
{
    u64 len = strlen(src);
    if(len < 4) return 0; 
    return strcmp(src + len - 4, suf)==0;

}


//write the instances set from a .txt 
//return the number of literals in the instance 
static u32 
writeInstanceLine(FILE *out, char *line)
{
    u32 count = 0;
    char *p = line;

    fprintf(out,"{ {");

    while (*p)
    {
        i32 idx, val;

        
        if (parseCondition(&p, &idx, &val)) 
        {
            if (count > 0) fprintf(out, ",");
            fprintf(out, "{%d,%d}", idx, val);
            count++;
        }
        //  next '&'
        while (*p && *p != '&') p++;
        if (*p == '&') p++;
    }

    fprintf(out, "}, %u}", count);

    return count;

}

static InstanceSizes
writeInstanceFromTXT(FILE *out, FILE *in)
{
    u32 maxLiterals = 0;
    u32 count = 0;

    fprintf(out, "InstanceList instanceSet[] = {\n");

    u32 literals;

    char line[512];
    while (1)
    {

        //check if we reached the EOF 
        if (!fgets(line, sizeof(line), in)) break; 


        line[strcspn(line, "\r\n")] = 0;

        //black line or no entry at all
        if(line[0]=='\0') break;


        fprintf(out, "    {%u, ", count++);
        literals = writeInstanceLine(out, line);
        fprintf(out, "},\n");

        if (literals == 0) { count--; break; }    // blank/empty line
        if (literals > maxLiterals) maxLiterals = literals;
    }

    fprintf(out, "};\n");
    return (InstanceSizes){ maxLiterals, count };
}



static u8 
writeIndexes(char **p, FILE *out) {
    skipSpaces(p);

    if (**p != 'a') return 0;
    (*p)++;

    if (**p != '(') return 0;
    (*p)++;

    u32 idx = parseInt(p);

    fprintf(out, "%d",idx);


    if (**p != ')') return 0;
    (*p)++;

    skipSpaces(p);

    return 1;

}

static u32 
writeColumnIndex(FILE *out, char *line, u32 size)
{
    u32 count =0;

    //skip the first column //
   while (*line && *line != ',') line++;
    
    while(line[0] != '\0')
    {
        skipSpaces(&line);
        if(line[0] != ',') break;
        line++; //skip the ','  
        skipSpaces(&line);
        if(line[0]=='y') break;
        fprintf(out, "#define _%d_IDXS    ", count + 1);
        writeIndexes(&line, out);
        fprintf(out, "\n");
        count++;
    }
    
    return count;

}


static u32 
writeInstanceCsvLine(FILE *out, char *line, u32 colCount)
{
    u32 count = 0;
    char *p = line;

    fprintf(out,"{ {");

    while (*p)
    {

        //leave the prediction column
        if( count + 1 >= colCount) break;  
        
        //skip the ','
    
        skipSpaces(&p);
        if(p[0] != ',') break;
        p++;


        
        //parse int
        skipSpaces(&p);
        u32 idx = parseInt(&p);
        
        //write condition 
        if(count > 0) fprintf(out,",");
        fprintf(out,"{_%d_IDXS, %d}", ++count, idx);


    }

    fprintf(out, "}, %u}", count);

    return count;

}

static InstanceSizes 
writeInstanceFromCSV(FILE *out, FILE *in)
{

    char line[512];

    //get the header lines 
    fgets(line,sizeof(line), in);

    u32 cols = writeColumnIndex(out, line, 512);
    u32 count = 0;
    u32 literals;


    fprintf(out, "\n\nInstanceList instanceSet[] = {\n");

    while (1)
    {

        //check if we reached the EOF 
        if (!fgets(line, sizeof(line), in)) break; 


        line[strcspn(line, "\r\n")] = 0;

        //black line or no entry at all
        if(line[0]=='\0') break;

        //parse the instance id 
        char *lines = line;
        skipSpaces(&lines);
        u32 id = parseInt(&lines);



        fprintf(out, "    {%u, ", id);
        literals = writeInstanceCsvLine(out, lines, cols);
        fprintf(out, "},\n");

        count++;

    }

    fprintf(out, "};\n");
    return (InstanceSizes){ cols, count };


    

}


static InstanceSizes 
writeInstance(FILE *out, FILE *in, char *filename)
{
    if(endsWith4(filename, ".txt")) return writeInstanceFromTXT(out,in);
    else if(endsWith4(filename, ".csv")) return writeInstanceFromCSV(out,in);

    return (InstanceSizes){0,0} ;
}


HeaderPatchValues writeSrc(FILE *out, FILE *in)
{
    HeaderPatchValues pv = {0};



    fprintf(out, "#include \"rules.h\"\n\n\n");
    fprintf(out, "Rule ruleset[] = {\n");

    char line[512];
    u32 count = 0;
    u32 maxCond = 0;
    u32 maxFeature = 0;

    while (fgets(line, sizeof(line), in)) 
    {
        // strip newline
        line[strcspn(line, "\r\n")] = 0;

        // parse rule id
        char *p = line;
        skipSpaces(&p);

        if (!(*p >= '0' && *p <= '9')) continue;
        int id = parseInt(&p);

        if (*p != ':') continue;
        p++;

        // find IF and THEN
        char *ifPart = strstr(p, "IF");
        char *thenPart = strstr(p, "THEN");

        if (!ifPart || !thenPart) continue;

        // split the condition part
        *thenPart = '\0';
        char *conds = ifPart + 2; // after "IF"

        fprintf(out, "    {%d, {", id);

        int condCount = 0;
        char *cp = conds;

        //  conditions
        while (*cp) {
            int idx, val;

            if (parseCondition(&cp, &idx, &val)) {
                if (condCount > 0) fprintf(out, ",");
                fprintf(out, "{%d,%d}", idx, val);

                if ((u32)idx > maxFeature)
                    maxFeature = idx;

                condCount++;
            }

            // move to next '&'
            while (*cp && *cp != '&') cp++;
            if (*cp == '&') cp++;
        }

        // get the result 
        i32 result = -1;

        char *rp = thenPart + 4; 

        while (*rp && *rp != 'y') rp++;

        if (*rp == 'y') {
            rp++;  // skip 'y'

            skipSpaces(&rp);

            if (*rp == '=') {
                rp++;  // skip '='
                skipSpaces(&rp);

                if (*rp >= '0' && *rp <= '9') {
                    result = parseInt(&rp);
                }
            }
        }

        //check if the parsing of the result is correct 
        //TODO: fix this , it fails on some input (white space isses)

        if (result == -1) {
            fprintf(stderr, "Could not parse result in line: %s\n", line);
        }

        fprintf(out, "}, %d, %d}", condCount, result);

        if ((u32)condCount > maxCond)
            maxCond = condCount;

        fprintf(out, ",\n");

        count++;
    }

    fprintf(out, "};\n\n");

    pv.maxCond = maxCond;
    pv.rulesCount = count;
    pv.maxFeature = maxFeature;

    return pv;
}

