#ifndef HELPERS
#define HELPERS


#include "types.h"
#include "bus.h"

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

// Retorna true se o estado da instrução fornecido terminou o estágio de execução
bool scoreboard_finished_executing(InstructionState state,
                                  InstructionBinary binary, 
                                  CPU_Configurations cpu_configs);

void update_rj_and_rk(int opcode, FunctionalUnitState *state, FunctionalUnit *register_status[]);

void clear_uf_state(FunctionalUnitState *uf_state);

// Retorna o resultado das operações. Em casos de branch, apenas retorna true ou false
int actually_execute(int opcode, int operand1, int operand2);

void update_finished_instructions(ScoreBoard *score_board, Byte *memory, int inst_count);

// Checa se pode enviar alguma instrução para write result
// Senão, continua a executar
void update_write_result(Bus *bus, 
                        Byte *memory, 
                        ScoreBoard *score_board, 
                        FunctionalUnit *functional_units, 
                        CPU_Configurations cpu_configs, 
                        int curr_cycle, 
                        int inst_count);

// Checa se pode enviar alguma instrução para execute
void update_execute(Bus *bus, 
                    FunctionalUnit *functional_units, 
                    ScoreBoard *score_board, 
                    int curr_cycle, 
                    int inst_count);

// Checa se pode enviar alguma instrução para read operands
void update_read_operands(Bus *bus, 
                          FunctionalUnit *functional_units, 
                          ScoreBoard *score_board, 
                          int curr_cycle, 
                          int count_instructions);

// Checa se pode enviar alguma instrução para issue
void update_issue(Bus *bus, 
                  FunctionalUnit *functional_units, 
                  ScoreBoard *score_board, 
                  InstructionRegister ir, 
                  CPU_Configurations cpu_configs,
                  int curr_cycle, 
                  int total_ufs, 
                  int inst_count);

// Checa se pode enviar alguma instrução para fetch
void update_fetch(Bus *bus,
                  Byte *memory, 
                  ScoreBoard *score_board, 
                  InstructionRegister *ir, 
                  int pc, 
                  int curr_cycle, 
                  int total_ufs, 
                  int instruction_count);



void print_instruction_status(InstructionState** instruction_states,
                              Byte inst_opcodes[],
                              int num_instructions);

void print_functional_unit_status(FunctionalUnitState* functional_unit_states, 
                                  int num_ufs);   

void print_result_register_status(FunctionalUnit* result_register_state[]);

void print_registers(int* registers);

void print_table(ScoreBoard* scoreboarding, int curr_cycle, Byte inst_opcodes[], int num_instructions, int num_ufs);

#endif