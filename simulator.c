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
  if (g_bus.ir_binary.flag == WRITE_TO_DESTINATION)
    g_instruction_register.binary = g_bus.ir_binary.data;

  if (g_bus.ir_pc.flag == WRITE_TO_DESTINATION)
    g_instruction_register.program_counter = g_bus.ir_pc.data;
}
void issue_instruction(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_ISSUE) continue;

    g_bus_buffer.ufs_data[0][uf_index].data = g_instruction_register.binary;
    printf("binary no issue: %d\n", g_instruction_register.binary);
    g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
    g_bus_buffer.ufs_data[0][uf_index].type = INSTRUCTION_BINARY;
  }
}
void read_operands(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_READ_OPERAND) continue;

    int binary = g_functional_units[uf_index].instruction_binary;
    
    int opcode = get_opcode_from_binary(binary);
    InstructionFormat format = get_inst_format_from_opcode(opcode);
    printf("index que entrou no continue read op: %d\n", uf_index);
    printf("binary: %d\n", binary);
    printf("opcode: %d\n", opcode);
    printf("format: %d\n", format);
    int operand1_index, operand2_index, operand2, imm;
    if (is_branch(opcode)){
      if (format == FORMAT_I){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2_index = get_rt_from_instruction_binary(binary);

        g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        g_bus_buffer.ufs_data[1][uf_index].data = g_registers[operand2_index];
        g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
      }
      else if (format == FORMAT_J){
        imm = get_imm_from_instruction_binary(binary);
        
        g_bus_buffer.ufs_data[0][uf_index].data = imm;
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;
      }
    }
    else{
      if (format == FORMAT_R){
        operand1_index = get_rt_from_instruction_binary(binary);
        operand2_index = get_rd_from_instruction_binary(binary);
        printf("operan1 index: %d\n", operand1_index);
        printf("operan2 index: %d\n", operand2_index);
        g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        g_bus_buffer.ufs_data[1][uf_index].data = g_registers[operand2_index];
        g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
      }
      else if (format == FORMAT_I){
        operand1_index = get_rt_from_instruction_binary(binary);
        operand2 = get_imm_from_instruction_binary(binary);
        printf("operand2: %d\n", operand2);
        g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        g_bus_buffer.ufs_data[1][uf_index].data = operand2;
        g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
      }
      else if (format == FORMAT_J){
        // Não sei quão certo isso está
        g_functional_units[uf_index].operand1 = get_imm_from_instruction_binary(binary);
      }
    }
  }
}
void execute(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int i = 0; i < total_ufs; i++){
    if (g_functional_units[i].status != CONTINUE_EXECUTE) continue;
      
    UF_TYPE type = g_functional_units[i].type;
    int cycles_to_complete;
    if (type == ADD_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_add;
    if (type == MUL_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_mul;
    if (type == INTEGER_UF) cycles_to_complete = g_cpu_configs.cycles_to_complete_integer;

    g_functional_units[i].current_cycle++;
    printf("current cycle dentro a uf: %d\n", g_functional_units[i].current_cycle);
    printf("cycles to complete: %d\n", cycles_to_complete);
    if (g_functional_units[i].current_cycle == cycles_to_complete){
      g_functional_units[i].current_cycle = 0;

      int binary = g_functional_units[i].instruction_binary;
      int opcode = get_opcode_from_binary(binary);
      int operand1 = g_functional_units[i].operand1;
      int operand2 = g_functional_units[i].operand2;
      int result = actually_execute(opcode, operand1, operand2);

      if (!is_branch(opcode))
        g_functional_units[i].operation_result = result;
      else if (result){
        g_functional_units[i].operation_result = g_program_counter + 4*get_imm_from_instruction_binary(binary);
      }
      else{
        g_functional_units[i].operation_result = 0; // Atribuindo 0 pois, para essa simulação, 0 não é um valor válido para o PC
      }
      // printf("opcode: %d\n", opcode);
      // printf("operand1: %d\n", operand1);
      // printf("operand2: %d\n", operand2);
      // printf("op result: %d\n", g_functional_units[i].operation_result);
      // printf("deu actually execute\n");
    }
  }
}
void write_result(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    FunctionalUnit uf = g_functional_units[uf_index];

    if (uf.status != CONTINUE_WRITE_RESULT) continue;

    printf("Deu continue write result\n");
    uf.status = STALL; // Aqui stall indica que não está em uso

    int binary = uf.instruction_binary;
    int opcode = get_opcode_from_binary(binary);
    if (!is_branch(opcode)){
      int result = get_destination_register_from_instruction(binary);

      g_bus_buffer.regs[result].data = uf.operation_result;
      g_bus_buffer.regs[result].flag = WRITE_TO_DESTINATION;
    }
    else{
      if (uf.operation_result != 0){
        g_bus_buffer.pc.data = uf.operation_result;
        g_bus_buffer.pc.flag = WRITE_TO_DESTINATION;
      }
    }
    // todo -> também tem que lidar com load e store
  }
}

/************************/

void update_scoreboard(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  update_finished_instructions(&g_score_board,
      g_instruction_count);
  
  printf("Terminou update_finished_instructions\n");
      
  update_write_result(&g_bus_buffer,
      g_memory,
      &g_score_board,
      g_cpu_configs,
      g_current_cycle,
      g_instruction_count);
  
  printf("Terminou update_write_result\n");
      
  update_execute(&g_bus_buffer,
      &g_score_board,
      g_current_cycle,
      g_instruction_count);
  
  printf("Terminou update_execute\n");
      
  update_read_operands(&g_bus_buffer,
      &g_score_board,
      g_current_cycle,
      total_ufs);
  
  printf("Terminou update_read_operands\n");
      
  update_issue(&g_bus_buffer,
      &g_score_board,
      g_instruction_register,
      g_current_cycle,
      total_ufs,
      g_instruction_count);
  
  printf("Terminou update_issue\n");
      
  update_fetch(&g_bus_buffer,
      g_memory,
      &g_score_board,
      &g_instruction_register,
      &g_program_counter,
      g_current_cycle,
      total_ufs,
      g_instruction_count);
  
  printf("Terminou update_fetch\n");
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
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    g_bus.ufs_data[0][uf_index].data = g_bus_buffer.ufs_data[0][uf_index].data;
    g_bus.ufs_data[0][uf_index].flag = g_bus_buffer.ufs_data[0][uf_index].flag;
    g_bus.ufs_data[0][uf_index].type = g_bus_buffer.ufs_data[0][uf_index].type;

    g_bus.ufs_data[1][uf_index].data = g_bus_buffer.ufs_data[1][uf_index].data;
    g_bus.ufs_data[1][uf_index].flag = g_bus_buffer.ufs_data[1][uf_index].flag;
    g_bus.ufs_data[1][uf_index].type = g_bus_buffer.ufs_data[1][uf_index].type;

    g_bus.ufs_state[uf_index] = g_bus_buffer.ufs_state[uf_index];
    // printf("uf state: %d\n", g_bus.ufs_state[uf_index]);
    // printf("uf state buffer: %d\n", g_bus_buffer.ufs_state[uf_index]);
    // Clear buffer
    g_bus_buffer.ufs_data[0][uf_index].flag = IGNORE;
    g_bus_buffer.ufs_data[1][uf_index].flag = IGNORE;
    g_bus_buffer.ufs_state[uf_index] = STALL;
  }

  // IR
  g_bus.ir_binary.data = g_bus_buffer.ir_binary.data;
  g_bus.ir_binary.flag = g_bus_buffer.ir_binary.flag;

  g_bus.ir_pc.data = g_bus_buffer.ir_pc.data;
  g_bus.ir_pc.flag = g_bus_buffer.ir_pc.flag;

  g_bus_buffer.ir_binary.flag = IGNORE;
  g_bus_buffer.ir_pc.flag = IGNORE;
}

void receive_data_from_bus(){
  for (int i = 0; i < g_memory_size; i++){
    if (g_bus.memory[i].flag == WRITE_TO_DESTINATION)
      g_memory[i] = g_bus.memory[i].data;
  }
  for (int i = 0; i < NUM_REGISTERS; i++){
    if (g_bus.regs[i].flag == WRITE_TO_DESTINATION)
      g_registers[i] = g_bus.regs[i].data;
  }
  
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_bus.ufs_data[0][uf_index].flag == WRITE_TO_DESTINATION){
      UF_DataType type = g_bus.ufs_data[0][uf_index].type;
      if (type == OPERAND1)
        g_functional_units[uf_index].operand1 = g_bus.ufs_data[0][uf_index].data;
      else if (type == OPERAND2)
        g_functional_units[uf_index].operand2 = g_bus.ufs_data[0][uf_index].data;
      else if (type == INSTRUCTION_BINARY)
        g_functional_units[uf_index].instruction_binary = g_bus.ufs_data[0][uf_index].data;
    }
    if (g_bus.ufs_data[1][uf_index].flag == WRITE_TO_DESTINATION){
      UF_DataType type = g_bus.ufs_data[1][uf_index].type;
      if (type == OPERAND1)
        g_functional_units[uf_index].operand1 = g_bus.ufs_data[1][uf_index].data;
      else if (type == OPERAND2){
        g_functional_units[uf_index].operand2 = g_bus.ufs_data[1][uf_index].data;
      }
      else if (type == INSTRUCTION_BINARY)
        g_functional_units[uf_index].instruction_binary = g_bus.ufs_data[1][uf_index].data;
    }

    g_functional_units[uf_index].status = g_bus.ufs_state[uf_index];
  }

}

void run_one_cycle(FILE *output){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  printf("Começou run one cycle\n");

  g_current_cycle++;
  

  write_result();
  printf("Terminou write_result\n");
  execute();
  printf("Terminou execute\n");
  read_operands();
  printf("Terminou read_operands\n");
  issue_instruction();
  printf("Terminou issue_instruction\n");
  fetch_next_instruction();
  printf("Terminou fetch_next_instruction\n");

  update_scoreboard();
  printf("Terminou update_scoreboard\n");
  send_data_to_bus();
  printf("Terminou send_data_to_bus\n");
  receive_data_from_bus();
  printf("Terminou receive_data_from_bus\n");

  for (int i = 0; i < NUM_REGISTERS; i++){
    printf("r%d:%d  ", i, g_registers[i]);
  }
  printf("\n");

  for (int i = 0; i < total_ufs; i++){
    printf("Printando uf de index %d\n", i);
    print_uf(g_functional_units[i]);
  }

  // Imprimindo tabelas, não fiel à simulação
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
  memset(g_registers, 0, sizeof(g_registers));
  g_instruction_register.program_counter = PROGRAM_FIRST_ADDRESS; 

  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &input_file_name, &input_file, &output_stream)){
    
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, &g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, &g_bus, &g_bus_buffer, g_cpu_configs, g_instruction_count, g_memory_size);
      init_scoreboard(&g_score_board);
      init_functional_units(g_functional_units, g_cpu_configs);

      printf("printando instructions binaries\n");
      for (int i=0; i<g_instruction_count; i++){
        printf("instruction[%d]: %d\n", i, get_instruction_from_memory(i, g_memory));
        printf("opcode: %d\n", get_opcode_from_binary(get_instruction_from_memory(i, g_memory)));
      }

      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura da linha de comando.\n");
  }

  free_memory(input_file, output_stream, &g_bus, &g_bus_buffer, &g_score_board, &g_functional_units);

  return 0;

}
