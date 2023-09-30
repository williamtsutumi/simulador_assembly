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
#include "headers/bus.h"

CPU_Configurations g_cpu_configs;
FunctionalUnit *g_functional_units;
ScoreBoard g_score_board;
Bus g_bus;

Byte *g_memory;
int g_memory_size = 200;

int g_registers[NUM_REGISTERS];
int g_instruction_count = 0;

int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

/* Execução nas unidades funcionais */

void fetch_next_instruction(){
  for (int i = 0; i < g_instruction_count; i++){
    if (g_score_board.instructions_states[i].current_state != FETCH) continue;

    int curr_inst_index = (g_instruction_register.program_counter - PROGRAM_FIRST_ADDRESS) / 4;
    // printf("i: %d\n", i);
    // printf("curr inst index: %d\n", curr_inst_index);
    if (i == curr_inst_index) continue; // A instrução já foi fetchada
    
    int inst_index = i;
    g_instruction_register.binary = get_instruction_from_memory(inst_index, g_memory);
    g_instruction_register.program_counter = g_program_counter;
    // add_pulse(&g_bus, 
    // new_data_pulse(get_instruction_from_memory(inst_index, g_memory), &(g_instruction_register.binary), sizeof(int)));
    // add_pulse(&g_bus, 
    // new_data_pulse(g_program_counter, &(g_instruction_register.program_counter), sizeof(int)));

    g_program_counter += 4;
    // add_pulse(&g_bus, 
    // new_data_pulse(g_program_counter + 4, &(g_program_counter), sizeof(int)));
    break;
  }
}

void issue_instruction(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_ISSUE) continue;
    yellow();
    printf("Efetivamente dando issue na uf de idx %d\n", uf_index);
    reset();
    add_pulse(&g_bus, 
      new_data_pulse(g_instruction_register.binary, &(g_functional_units[uf_index].instruction_binary), sizeof(InstructionBinary)));

    break;
    
  }
}
void read_operands(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_READ_OPERAND) continue;

    InstructionBinary binary = g_functional_units[uf_index].instruction_binary;
    int opcode = get_opcode_from_binary(binary);

    InstructionFormat format = get_inst_format_from_opcode(opcode);
    // printf("uf idx que entrou no continue read op: %d\n", uf_index);
    // printf("binary: %u\n", binary);
    // printf("opcode: %d\n", opcode);
    // printf("format: %d\n", format);
    // printf("is branch: %d\n", is_branch(opcode));
    int operand1_index, operand2_index, operand2, imm;
    if (is_branch(opcode)){
      if (format == FORMAT_I){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2_index = get_rt_from_instruction_binary(binary);
        red();
        printf("operan1 index: %d\n", operand1_index);
        printf("operan2 index: %d\n", operand2_index);
        reset();
        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));

        //g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        //g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        //g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand2), sizeof(int)));

        //g_bus_buffer.ufs_data[1][uf_index].data = g_registers[operand2_index];
        //g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        //g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
      }
      else if (format == FORMAT_J){
        imm = get_imm_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_data_pulse(imm, &(g_functional_units[uf_index].operand1), sizeof(int)));
        
        //g_bus_buffer.ufs_data[0][uf_index].data = imm;
        //g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        //g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;
      }
    }
    else{
      if (format == FORMAT_R){
        operand1_index = get_rs_from_instruction_binary(binary);
        operand2_index = get_rt_from_instruction_binary(binary);
        

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));
        
        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand2_index]), &(g_functional_units[uf_index].operand2), sizeof(int)));

        /*
        g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        g_bus_buffer.ufs_data[1][uf_index].data = g_registers[operand2_index];
        g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
        */
      }
      else if (format == FORMAT_I){
        operand1_index = get_rt_from_instruction_binary(binary);
        operand2 = get_imm_from_instruction_binary(binary);

        add_pulse(&g_bus, 
        new_pulse(&(g_registers[operand1_index]), &(g_functional_units[uf_index].operand1), sizeof(int)));
        
        add_pulse(&g_bus, 
        new_data_pulse(operand2, &(g_functional_units[uf_index].operand2), sizeof(int)));

        printf("operand2: %d\n", operand2);

        /*
        g_bus_buffer.ufs_data[0][uf_index].data = g_registers[operand1_index];
        g_bus_buffer.ufs_data[0][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[0][uf_index].type = OPERAND1;

        g_bus_buffer.ufs_data[1][uf_index].data = operand2;
        g_bus_buffer.ufs_data[1][uf_index].flag = WRITE_TO_DESTINATION;
        g_bus_buffer.ufs_data[1][uf_index].type = OPERAND2;
        */
      }
      else if (format == FORMAT_J){ // tem dois if FORMAT_J ?
        // Não sei quão certo isso está
        g_functional_units[uf_index].operand1 = get_imm_from_instruction_binary(binary);
        assert(false);
      }
    }
    // printf("operan1 index: %d\n", operand1_index);
    // printf("operan2 index: %d\n", operand2_index);
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
    printf("Continuando a execução da uf de idx %d\n", i);
    printf("current cycle dentro a uf: %d\n", g_functional_units[i].current_cycle);
    printf("cycles to complete: %d\n", cycles_to_complete);
    if (g_functional_units[i].current_cycle == cycles_to_complete){
      g_functional_units[i].current_cycle = 0;

      InstructionBinary binary = g_functional_units[i].instruction_binary;
      int opcode = get_opcode_from_binary(binary);
      int operand1 = g_functional_units[i].operand1;
      int operand2 = g_functional_units[i].operand2;
      printf("opcode: %d\n", opcode);
      printf("operand1: %d\n", operand1);
      printf("operand2: %d\n", operand2);
      int result = actually_execute(opcode, operand1, operand2);
      printf("op result: %d\n", g_functional_units[i].operation_result);

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
    }
  }
}
void write_result(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;
  for (int uf_index = 0; uf_index < total_ufs; uf_index++){
    if (g_functional_units[uf_index].status != CONTINUE_WRITE_RESULT) continue;

    g_functional_units[uf_index].status = STALL; // Aqui stall indica que não está em uso

    InstructionBinary binary = g_functional_units[uf_index].instruction_binary;
    int opcode = get_opcode_from_binary(binary);
    if (is_branch(opcode)){
      if (g_functional_units[uf_index].operation_result != 0){
        g_program_counter = g_functional_units[uf_index].operation_result;
        // add_pulse(&g_bus, 
        // new_pulse(&g_functional_units[uf_index].operation_result, &g_program_counter, sizeof(int)));
      }
    }
    else if(is_memory(opcode)){
      // TODO
    }
    else{
      int result = get_destination_register_from_instruction(binary);

      add_pulse(&g_bus, 
      new_pulse(&g_functional_units[uf_index].operation_result, &g_registers[result], sizeof(int)));
    }
  }
}

/************************/

void update_scoreboard(){
  int total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_mul_ufs + g_cpu_configs.size_integer_ufs;

  update_finished_instructions(&g_score_board,
      g_memory,
      g_instruction_count);
  
  printf("Terminou update_finished_instructions\n");
      
  update_write_result(&g_bus,
      g_memory,
      &g_score_board,
      g_functional_units,
      g_cpu_configs,
      g_current_cycle,
      g_instruction_count);
  
  printf("Terminou update_write_result\n");
      
  update_execute(&g_bus,
      g_functional_units,
      &g_score_board,
      g_current_cycle,
      g_instruction_count);
  
  printf("Terminou update_execute\n");
      
  update_read_operands(&g_bus,
      g_functional_units,
      &g_score_board,
      g_current_cycle,
      g_instruction_count);
  
  printf("Terminou update_read_operands\n");
      
  update_issue(&g_bus,
      g_functional_units,
      &g_score_board,
      g_instruction_register,
      g_current_cycle,
      total_ufs,
      g_instruction_count);
  
  printf("Terminou update_issue\n");
      
  update_fetch(&g_bus,
      g_memory,
      &g_score_board,
      &g_instruction_register,
      g_program_counter,
      g_current_cycle,
      total_ufs,
      g_instruction_count);
  
  printf("Terminou update_fetch\n");
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
  dispatch_pulses(&g_bus);

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

bool program_has_exited(){
  int output = true;
  for (int i = 0; i < g_instruction_count - 1; i++){
    if (g_score_board.instructions_states[i].current_state != FINISHED){
      output = false;
      break;
    }
  }
  // Se a exit foi fetchada
  if (g_score_board.instructions_states[g_instruction_count - 1].current_state != FETCH) output = false;

  return output;
}

void run_simulation(FILE *output){
  while (!program_has_exited()){
    getchar();
    run_one_cycle(output);
  }

  printf("Program Exited.\n");
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

  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &input_file_name, &input_file, &output_stream)){
    
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, &g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, g_cpu_configs, g_instruction_count, g_memory_size);
      init_scoreboard(&g_score_board);
      init_functional_units(g_functional_units, g_cpu_configs);

      printf("printando instructions binaries\n");
      for (int i=0; i<g_instruction_count; i++){
        printf("instruction[%u]: %d, opcode: %d\n", i, get_instruction_from_memory(i, g_memory), get_opcode_from_binary(get_instruction_from_memory(i, g_memory)));
      }

      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura da linha de comando.\n");
  }

  printf("A\n");
  free_memory(input_file, output_stream, &g_score_board, &g_functional_units);

  return 0;

}
