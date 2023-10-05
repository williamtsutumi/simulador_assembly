#ifndef HELPERS
#define HELPERS


#include "types.h"
#include "bus.h"
#include "scoreboard.h"

/* Utilidades */

int get_opcode_from_binary(InstructionBinary instruction);

InstructionFormat get_inst_format_from_opcode(int opcode);

char *get_inst_name_from_opcode(Byte op_code);

// Retorna o índice (no array de registradores) do registrador de destino
int get_destination_register_from_instruction(InstructionBinary instruction);


// instruction_index se refere à i-ésima instrução - 1
// primeira instrução => instruction_index 0
InstructionBinary get_instruction_from_memory(int instruction_index, Byte *mem);

unsigned int get_data_from_memory(int data_index, Byte *mem);

int get_rt_from_instruction_binary(InstructionBinary binary);

int get_rs_from_instruction_binary(InstructionBinary binary);

int get_rd_from_instruction_binary(InstructionBinary binary);

int get_imm_from_instruction_binary(InstructionBinary binary);

bool is_branch(int opcode);

bool is_conditional_branch(int opcode);

bool is_memory(int opcode);


/* Debug */

void print_uf(FunctionalUnit uf);


/* Utilidades Scoreboard */

// Retorna true se a unidade funcional fornecida terminou o estágio de execução
bool uf_finished_executing(FunctionalUnit *uf,
                            CPU_Configurations cpu_configs);


// Retorna o resultado das operações. Em casos de branch, apenas retorna true ou false
int actually_execute(int opcode, int operand1, int operand2);

UF_TYPE get_uf_type_from_instruction(InstructionBinary instruction);

void get_operands_register_from_instruction(InstructionBinary instruction, int* op1, int* op2);

int get_binary_subnumber(InstructionBinary instruction, int start_bit, int end_bit);





#endif