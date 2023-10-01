#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H


void malloc_memory(FunctionalUnit **functional_units, 
                  ScoreBoard *score_board,
                  CPU_Configurations cpu_configs, 
                  int num_instructions,
                  int memory_size);

void init_scoreboard(ScoreBoard *score_board);


void init_functional_units(FunctionalUnit *functional_units,
                          CPU_Configurations cpu_configs);



#endif