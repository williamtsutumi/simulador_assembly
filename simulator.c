#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#include "headers/assembly_parser.h"
#include "headers/types.h"
#include "headers/helpers.h"
#include "headers/memory_management.h"

CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;
Bus g_bus_buffer;

Byte *g_memory;
int g_memory_size = 200;

int g_registers[NUM_REGISTERS];
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

/* Estágios do pipeline */

void fetch_next_instruction(){

}
void issue_instruction(){
  
}
void read_operands(){
  
}
void execute(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int i = 0; i < total_ufs; i++){
    if (g_functional_units[i].status == CONTINUE_EXECUTE){
      UF_TYPE type = g_functional_units[i].type;
      int cycles_to_complete;
      if (type == ADD_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_add;
      if (type == MUL_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_mul;
      if (type == INTEGER_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_integer;

      g_functional_units[i].current_cycle++;
      if (g_functional_units[i].current_cycle == cycles_to_complete){
        int opcode = get_opcode_from_binary(g_functional_units[i].instruction_binary);
        int operand1 = g_functional_units[i].operand1;
        int operand2 = g_functional_units[i].operand2;
        g_functional_units[i].operation_result = actually_execute(opcode, operand1, operand2);
      }
    }
  }
}
void write_result(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_functional_units[i].status == CONTINUE_WRITE_RESULT){
      g_functional_units[i].status = STALL; // Aqui stall indica que está livre
      
      int result = get_destination_register_from_instruction(g_functional_units[i].instruction_binary);

      g_bus_buffer.regs[result].data = g_functional_units[i].operation_result;
      g_bus_buffer.regs[result].flag = WRITE_TO_DESTINATION;
      printf("VEIO AQUI\n");
    }
  }
}

/************************/

void update_scoreboard(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  update_write_result(&g_bus_buffer, g_memory, &g_score_board, g_cpu_configs, g_current_cycle, g_instruction_count);
  update_execute(&g_score_board, g_current_cycle, g_instruction_count);
  update_read_operands(&g_score_board, g_current_cycle, total_ufs);
  update_issue(&g_score_board, g_instruction_register, g_current_cycle, total_ufs);
  update_fetch(g_memory, &g_score_board, &g_instruction_register, &g_program_counter, g_current_cycle, total_ufs);
}

// Envia para o barramento tudo o que está no buffer do barramento e reseta a flag do buffer
void send_data_to_bus(){
  for (int i = 0; i < g_memory_size; i++){
    g_bus.memory[i].data = g_bus_buffer.memory[i].data;
    g_bus.memory[i].flag = g_bus_buffer.memory[i].flag;
    // Clear buffer
    g_bus_buffer.memory[i].flag = IGNORE;
  }
  
  for (int i = 0; i < NUM_REGISTERS; i++){
    g_bus.regs[i].data = g_bus_buffer.regs[i].data;
    g_bus.regs[i].flag = g_bus_buffer.regs[i].flag;
    // Clear buffer
    g_bus_buffer.regs[i].flag = IGNORE;
  }

  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    g_bus.ufs_data[i].data = g_bus_buffer.ufs_data[i].data;
    g_bus.ufs_data[i].flag = g_bus_buffer.ufs_data[i].flag;
    g_bus.ufs_data[i].type = g_bus_buffer.ufs_data[i].type;

    g_bus.ufs_state[i] = g_bus_buffer.ufs_state[i];
    // Clear buffer
    g_bus_buffer.ufs_data[i].flag = IGNORE;
  }
}

void receive_data_from_bus(){
  for (int i = 0; i < g_memory_size; i++){
    if (g_bus.memory[i].flag == WRITE_TO_DESTINATION)
      g_memory[i] = g_bus.memory[i].data;
  }
  for (int i = 0; i < NUM_REGISTERS; i++){
    printf("flag: %d\n", g_bus.regs[i].flag);
    if (g_bus.regs[i].flag == WRITE_TO_DESTINATION)
      g_registers[i] = g_bus.regs[i].data;
  }
  printf("Chegou aqui\n");

  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int i = 0; i < total_ufs; i++){
    if (g_bus.ufs_data[i].flag == WRITE_TO_DESTINATION){
      UF_DataType type = g_bus.ufs_data[i].type;
      if (type == OPERAND1)
        g_functional_units[i].operand1 = g_bus.ufs_data[i].data;
      else if (type == OPERAND2)
        g_functional_units[i].operand2 = g_bus.ufs_data[i].data;
      else if (type == INSTRUCTION_BINARY)
        g_functional_units[i].instruction_binary = g_bus.ufs_data[i].data;
    }

    g_functional_units[i].status = g_bus_buffer.ufs_state[i];
  }
}

void run_one_cycle(FILE *output){
  g_current_cycle++;

  printf("Comecou run one cycle\n");
  update_scoreboard();
  printf("Deu update scoreboard\n");
  send_data_to_bus();
  printf("Deu send data to bus\n");
  receive_data_from_bus();
  printf("Deu receive data from bus\n");

  write_result();
  execute();
  // Possívelmente não vai ter essas três
  // read_operands();
  // issue_instruction();
  // fetch_next_instruction();

  // Imprimindo tabelas, não fiel à simulação
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  Byte inst_opcodes[g_instruction_count];
  for (int i = 0; i < g_instruction_count; i++){
    int inst = get_instruction_from_memory(i, g_memory);
    int opcode = get_opcode_from_binary(inst);
    inst_opcodes[i] = opcode;
  }
  print_table(&g_score_board, g_current_cycle, inst_opcodes, g_instruction_count, total_ufs);
  // ****************************************

}

void run_simulation(FILE *output){
  bool has_active_instruction = true;
  while (has_active_instruction){
    getchar();
    run_one_cycle(output);
  }
}

bool read_args(int argc, char *argv[], char **input_file_name, FILE **input_file, FILE **output_stream){
  for(int i = 1; i < argc; i+=2){

    if(strcmp(argv[i], "-p") == 0){
      *input_file_name = argv[i+1];
    }
    else if(strcmp(argv[i], "-m") == 0){
      // Validando se é um número
      if (strspn(argv[i+1], "0123456789") != strlen(argv[i+1])) return false;

      g_memory_size = atoi(argv[i+1]);
    }
    else if(strcmp(argv[i], "-o") == 0){
      *output_stream = fopen(argv[i+1], "w");
    }
  }

  g_memory = (Byte*)(malloc(sizeof(Byte)* (PROGRAM_FIRST_ADDRESS + g_memory_size*4)));
  *input_file = fopen(*input_file_name, "r");
  return *input_file != NULL;
}

int main(int argc, char *argv[])
{
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &input_file_name, &input_file, &output_stream)){
    
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, &g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, &g_bus, &g_bus_buffer, g_cpu_configs, g_instruction_count, g_memory_size);
      init_scoreboard(&g_score_board);

      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura da linha de comando.\n");
  }

  free_memory(input_file, output_stream, &g_bus, &g_bus_buffer, &g_score_board, &g_functional_units);

  return 0;

}
