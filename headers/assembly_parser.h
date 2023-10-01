#ifndef ASSEMBLY_PARSER
#define ASSEMBLY_PARSER

#include "types.h"


bool parse_assembly(FILE *input,
                    FILE *output, 
                    CPU_Configurations *cpu_configs, 
                    int *instruction_count, 
                    Byte *memory, 
                    int memory_size);

#endif