#ifndef TYPES
#define TYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

// Símbolos para leitura da entrada
#define ADD "add "
#define ADD_OPCODE 0
#define ADDI "addi "
#define ADDI_OPCODE 1
#define SUB "sub "
#define SUB_OPCODE 2
#define SUBI "subi "
#define SUBI_OPCODE 3
#define MUL "mul "
#define MUL_OPCODE 4
#define DIV "div "
#define DIV_OPCODE 5
#define AND "and "
#define AND_OPCODE 6
#define OR "or "
#define OR_OPCODE 7
#define NOT "not "
#define NOT_OPCODE 8
#define BLT "blt "
#define BLT_OPCODE 9
#define BGT "bgt "
#define BGT_OPCODE 10
#define BEQ "beq "
#define BEQ_OPCODE 11
#define BNE "bne "
#define BNE_OPCODE 12
#define J "j "
#define J_OPCODE 13
#define LW "lw "
#define LW_OPCODE 14
#define SW "sw "
#define SW_OPCODE 15
#define EXIT "exit"
#define EXIT_OPCODE 16
#define INSTRUCTION_NAMES { ADD, ADDI, SUB, SUBI, MUL, DIV, AND, OR, NOT, BLT, BGT, BEQ, BNE, J, LW, SW, EXIT }

// Símbolos para leitura das configurações
#define UF_SYMBOL "UF"
#define INST_SYMBOL "INST"
#define ADD_SYMBOL "add"
#define MUL_SYMBOL "mul"
#define INTEGER_SYMBOL "integer"
#define CONFIG_SYMBOLS { ADD_SYMBOL, MUL_SYMBOL, INTEGER_SYMBOL }


#define MAX_NUM_ROWS_TABLE 100
#define NUM_REGISTERS 32
#define PROGRAM_FIRST_ADDRESS 400

// Acho que essas capacities não vão ser usadas por causa do funcionamento dessa cpu

// #define FETCH_CAPACITY 1
// #define ISSUE_CAPACITY 1
// #define READ_OPERANDS_CAPACITY 1
// #define EXECUTE_CAPACITY 1 
#define WRITE_RESULT_CAPACITY 1

/* Representação de Byte para a memória simulada */
typedef unsigned char Byte;

typedef unsigned InstructionBinary;

typedef enum InstructionFormat {
    FORMAT_R,
    FORMAT_I,
    FORMAT_J,
    FORMAT_INVALID
} InstructionFormat;

/* TYPES para leitura do input */

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

/* TYPES da representação das unidades funcionais */

typedef enum {
    INTEGER_UF,
    MUL_UF,
    ADD_UF,
    NOT_APPLIED_UF
} UF_TYPE;

// Indica o que a unidade funcional deve fazer: continuar sua execução ou permanecer parado
// A uf recebe esse status do scoreboard em todo ciclo
typedef enum FunctionalUnitStatus {
    STALL,
    CONTINUE_ISSUE,
    STALL_ISSUE,
    CONTINUE_READ_OPERAND,
    CONTINUE_EXECUTE,
    CONTINUE_WRITE_RESULT,
} FunctionalUnitStatus;

    typedef struct FunctionalUnit
    {
        UF_TYPE type;
        int type_index;

        FunctionalUnitStatus status;

        InstructionBinary instruction_binary;
        int current_cycle;
        int operand1;
        int operand2;
        int operation_result;
    } FunctionalUnit;

/* TYPES do scoreboarding */

// Indica qual estágio do pipeline a instrução está
typedef enum InstructionStateType {
    FETCH,
    ISSUE,
    READ_OPERANDS,
    EXECUTE,
    WRITE_RESULT,
    FINISHED
} InstructionStateType;

// Estado da instrução, controle realizado pelo scoreboard
typedef struct InstructionState {
    // Campos que não existem de fato no scoreboard, são usados para imprimir as tabelas
    int fetch;
    int issue;
    int read_operands;
    int start_execute;
    int finish_execute;
    int write_result;

    // Índice no array de FunctionalUnitState onde a instrução está
    // (não gostei de ter esse cara aqui)
    int uf_index;
    InstructionStateType current_state;
} InstructionState;

// Estado da unidade funcional, controle realizado pelo scoreboard
typedef struct FunctionalUnitState
{
    UF_TYPE type;
    // se tem vários do mesmo tipo, type_index é o índice q diferencia elas
    int type_index;
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

        bool can_fetch;
        bool has_conditional_branch;
    } ScoreBoard;



/* GERAL */

typedef struct InstructionRegister{
    InstructionBinary binary;
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