#ifndef TYPES
#define TYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>


#define MAX_NUM_INSTRUCTIONS 50
#define MAX_NUM_ROWS_TABLE 100
#define NUM_REGISTERS 32
#define MAX_QUEUE_SIZE 8

/* TYPES para leitura do input */

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

/* TYPES da representação das unidades funcionais */

// NÃO MEXER
typedef enum {
    INTEGER_UF,
    MUL_UF,
    ADD_UF,
    NOT_APPLIED_UF
} UF_TYPE;

    typedef struct FunctionalUnit
    {
        UF_TYPE type;
        int current_cycle;
    } FunctionalUnit;

/* TYPES do scoreboarding */

typedef enum InstructionStateType {
    FETCH,
    ISSUE,
    READ_OPERANDS,
    EXECUTE,
    WRITE_RESULT
} InstructionStateType;

typedef struct InstructionState {
    int fetch;
    int issue;
    int read_operands;
    int execute;
    int write_result;

    InstructionStateType current_state;
} InstructionState;

typedef struct FunctionalUnitState
{
    UF_TYPE type;
    int op, fi, fj, fk, qj, qk;
    bool busy, rj, rk;
} FunctionalUnitState;

    typedef struct ScoreBoard
    {
        FunctionalUnitState *ufs_states;
        InstructionState *instructions_states;
        // Representa qual unidade funcional escreverá em qual registrador.
        // Qualquer instrução deve dar stall caso o registrador de destino
        // já esteja sendo sofrendo escrita por outra instrução.
        FunctionalUnit* result_register_state[NUM_REGISTERS];

        bool can_issue_next_instruction;
    } ScoreBoard;

/* TYPES do barramento */

// Indica o que deve um componente deve fazer: continuar sua execução ou permanecer parado
typedef enum ControlSignal {
    CONTINUE,
    STALL
} ControlSignal;

// Indica se a informação no barramento deve ser lido ou não
typedef enum SignalFlag {
    IGNORE,
    WRITE_TO_DESTINATION
} SignalFlag;

    // Indica um dado se movendo de um componente a outro, representa uma parte do barramento
    typedef struct DataSignal {
        int data;
        SignalFlag flag;
    } DataSignal;

        typedef struct Bus {
            DataSignal regs[32]; // Informação sendo enviada aos registradores
            DataSignal *ufs_data; // Informação sendo enviada às unidades funcionais
            ControlSignal instruction_register;
            ControlSignal *ufs_state;
        } Bus;



/* GERAL */

typedef struct InstructionRegister{
    int binary;
    int program_counter;
} InstructionRegister;

typedef struct CPU_Configurations
{
  int size_add_ufs;
  int size_mul_ufs;
  int size_integer_ufs;
  int cycles_to_complete_add;
  int cycles_to_complete_mul;
  int cycles_to_complete_integer;
} CPU_Configurations;

#endif