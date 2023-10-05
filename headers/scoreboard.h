#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "types.h"
#include "bus.h"

typedef struct ScoreBoard
    {
        FunctionalUnitState *ufs_states;
        InstructionState *instructions_states;
        // Representa qual unidade funcional escreverá em qual registrador.
        // Qualquer instrução deve dar stall caso o registrador de destino
        // já esteja sendo sofrendo escrita por outra instrução.
        FunctionalUnit* result_register_state[NUM_REGISTERS];

        bool can_fetch;
        bool has_conditional_branch;
    } ScoreBoard;

void update_scoreboard(ScoreBoard*,
    Bus*,
    Byte*,
    FunctionalUnit*, 
    CPU_Configurations, 
    int, 
    int, 
    InstructionRegister,
    int);


#endif