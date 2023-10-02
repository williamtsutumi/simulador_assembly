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

int g_total_ufs;
int g_current_cycle = 0;
int g_program_counter = PROGRAM_FIRST_ADDRESS; // PC
InstructionRegister g_instruction_register; // IR

/* Execução nas unidades funcionais */

void fetch(){

  if(g_score_board.can_fetch == false) return;

  for(int i = 0; i < g_instruction_count; i++){
    InstructionStateType state = g_score_board.instructions_states[i].current_state;
    InstructionBinary instruction = get_instruction_from_memory(i, g_memory);
    int opcode = get_opcode_from_binary(instruction);
   
    // não pode realizar fetch, porque tem um salto condicional no pipeline.
    if(FETCH <= state && state < WRITE_RESULT && is_branch(opcode)){
      return;
    }
  }

  InstructionBinary instruction = get_data_from_memory(g_program_counter/4, g_memory);
  int curr_inst_index = (g_program_counter - PROGRAM_FIRST_ADDRESS) / 4;
  if(curr_inst_index != 0){
    printf("inst %d %d\n", curr_inst_index, g_score_board.instructions_states[curr_inst_index-1].current_state);
  }
  if(curr_inst_index != 0 && g_score_board.instructions_states[curr_inst_index-1].current_state == ISSUE) return;
  
  if(get_opcode_from_binary(instruction) == J_OPCODE){
    g_program_counter = get_imm_from_instruction_binary(instruction);
    g_score_board.instructions_states[curr_inst_index].fetch =
    g_score_board.instructions_states[curr_inst_index].write_result = g_current_cycle;

    g_score_board.instructions_states[curr_inst_index].current_state = WRITE_RESULT;
  }
  else if(get_opcode_from_binary(instruction) == EXIT_OPCODE){
    g_score_board.instructions_states[curr_inst_index].fetch = g_current_cycle;
    g_score_board.can_fetch = false;
  }
  else{
    g_score_board.instructions_states[curr_inst_index].current_state = ISSUE;
    g_score_board.instructions_states[curr_inst_index].fetch = g_current_cycle;
    g_program_counter += 4;
    
  } 
}

bool issue_instruction(InstructionBinary instruction, int inst_program_counter){

  int op = get_opcode_from_binary(instruction);
  int dest = get_destination_register_from_instruction(instruction);

  // fetchou exit
  if(dest == -1){
    return false;
  }
  int src1, src2;
  printf("%d\n", instruction);
  get_operands_register_from_instruction(instruction, &src1, &src2);



  printf("sources: %d %d, dest: %d\n", src1, src2, dest);

  for (int uf_index = 0; uf_index < g_total_ufs; uf_index++){
    UF_TYPE uf_type = get_uf_type_from_index(uf_index, g_cpu_configs);

    bool can_use_uf = !g_score_board.ufs_states[uf_index].busy
                    && g_score_board.result_register_state[dest] == NULL
                    && can_uf_execute_instruction(uf_type, op);

    if(can_use_uf){
      g_score_board.ufs_states[uf_index].busy = true;
      g_score_board.ufs_states[uf_index].op = op;
      g_score_board.ufs_states[uf_index].fi = dest;
      g_score_board.ufs_states[uf_index].fj = src1;
      g_score_board.ufs_states[uf_index].fk = src2; 
      g_score_board.ufs_states[uf_index].qj = src1 != -1 ? g_score_board.result_register_state[src1] : NULL;
      g_score_board.ufs_states[uf_index].qk = src2 != -1 ? g_score_board.result_register_state[src2] : NULL;
      g_score_board.ufs_states[uf_index].rj = g_score_board.ufs_states[uf_index].qj == NULL;
      g_score_board.ufs_states[uf_index].rk = g_score_board.ufs_states[uf_index].qk == NULL;
      g_score_board.result_register_state[dest] = &(g_functional_units[uf_index]);

      g_score_board.ufs_states[uf_index].inst_program_counter = inst_program_counter;

      add_pulse(&g_bus,
        new_data_pulse(
          instruction, &(g_functional_units[uf_index].instruction_binary), sizeof(InstructionBinary)
        )
      );

      dispatch_pulses(&g_bus);

      return true;
    }
  }
  return false;
}

void issue(){
  for(int i = 0; i < g_instruction_count; i++){
    // usar barramento pra isso
    InstructionBinary instruction = get_instruction_from_memory(i, g_memory);
  
    if(g_score_board.instructions_states[i].current_state == ISSUE){
      bool issued = issue_instruction(instruction, i);
      if(issued){
        g_score_board.instructions_states[i].issue = g_current_cycle;
        g_score_board.instructions_states[i].current_state = READ_OPERANDS;
      }
    }
  }
}
void read_operands(){

  for (int uf_index = 0; uf_index < g_total_ufs; uf_index++){
    int uf_instruction_idx = g_score_board.ufs_states[uf_index].inst_program_counter;
    if(uf_instruction_idx == -1) continue;


    if(g_score_board.instructions_states[uf_instruction_idx].current_state != READ_OPERANDS) continue;

    if(g_score_board.ufs_states[uf_index].rj && g_score_board.ufs_states[uf_index].rk){
      g_score_board.ufs_states[uf_index].rj = g_score_board.ufs_states[uf_index].rk = false;

      g_score_board.instructions_states[uf_instruction_idx].read_operands = g_current_cycle;

      int fj = g_score_board.ufs_states[uf_index].fj;
      int fk = g_score_board.ufs_states[uf_index].fk;

      actually_read_operands(fj, fk, g_registers, &g_functional_units[uf_index], &g_bus);
      
      g_score_board.instructions_states[uf_instruction_idx].current_state = EXECUTE;

    }
  }
}
void execute(){

  // todo: fazer FunctionalUnit.h

  for (int i = 0; i < g_total_ufs; i++){
    int uf_instruction_idx = g_score_board.ufs_states[i].inst_program_counter; 
    if(uf_instruction_idx == -1) continue;

    if(g_score_board.instructions_states[uf_instruction_idx].current_state != EXECUTE) continue;

    g_functional_units[i].current_cycle++;

    if(g_score_board.instructions_states[uf_instruction_idx].start_execute == -1){
      g_score_board.instructions_states[uf_instruction_idx].start_execute = g_current_cycle;
      g_score_board.instructions_states[uf_instruction_idx].finish_execute = g_current_cycle;
      
    }
    else{
      g_score_board.instructions_states[uf_instruction_idx].finish_execute++;
    }

    if(uf_finished_executing(&g_functional_units[i], g_cpu_configs)){
      g_functional_units[i].current_cycle = 0;
      
      g_score_board.instructions_states[uf_instruction_idx].current_state = WRITE_RESULT;
      // todo fazer essa função
      execute_instruction(&g_functional_units[i], g_score_board.ufs_states[i].op, g_program_counter, g_memory);
    }
  }
}

/*
    g_functional_units[i].current_cycle++;
  
    if (uf_finished_executing(&g_functional_units[i], g_cpu_configs)){
      g_functional_units[i].current_cycle = 0;

      InstructionBinary binary = g_functional_units[i].instruction_binary;
      int opcode = get_opcode_from_binary(binary);
      int operand1 = g_functional_units[i].operand1;
      int operand2 = g_functional_units[i].operand2;
      int result = actually_execute(opcode, operand1, operand2);

      if (!is_branch(opcode))
        g_functional_units[i].operation_result = result;
      else{
        if (result){
          if (opcode == J_OPCODE) g_functional_units[i].operation_result = operand1;
          else g_functional_units[i].operation_result = g_program_counter + 4*get_imm_from_instruction_binary(binary);
        }
        else{
          // Atribuindo 0 pois, para essa simulação, 0 não é um valor válido para o PC
          // Então, representa não atualizar o PC
          g_functional_units[i].operation_result = 0;
        }
      }
      // printf("opcode: %d\n", opcode);
      // printf("operand1: %d\n", operand1);
      // printf("operand2: %d\n", operand2);
      // printf("op result: %d\n", g_functional_units[i].operation_result);
    }
  }
}
*/
void write_result(){
  for (int uf_index = 0; uf_index < g_total_ufs; uf_index++){
    int uf_instruction_idx = g_score_board.ufs_states[uf_index].inst_program_counter;
    if(uf_instruction_idx == -1) continue;

    if(g_score_board.instructions_states[uf_instruction_idx].current_state != WRITE_RESULT) continue;
    bool dep_op1 = true;
    bool dep_op2 = true;

    for(int i = 0; i < g_total_ufs; i++){
      dep_op1 &= g_score_board.ufs_states[i].rj == false ||
                    g_score_board.ufs_states[i].fj != g_score_board.ufs_states[uf_index].fi;

      dep_op2 &= g_score_board.ufs_states[i].rk == false ||
                    g_score_board.ufs_states[i].fk != g_score_board.ufs_states[uf_index].fi;
    }
    if(dep_op1 && dep_op2){
        // Aqui stall indica que não está em uso
        g_score_board.instructions_states[uf_instruction_idx].current_state = STALL;
        g_score_board.instructions_states[uf_instruction_idx].write_result = g_current_cycle;

        int destination = g_score_board.ufs_states[uf_index].fi;

        write_instruction_result(destination, &g_functional_units[uf_index], g_registers, &g_bus);

        
        
        for(int i = 0; i < g_total_ufs; i++){
          if(g_score_board.ufs_states[i].qj == &g_functional_units[uf_index]){
            g_score_board.ufs_states[i].rj = true;
            g_score_board.ufs_states[i].rk = true;
          }
        }
        clear_uf_state(&g_score_board.ufs_states[uf_index]);
                
    }
  }
}


/************************************/



void run_one_cycle(FILE *output){
  printf("Começou run one cycle\n");

  g_current_cycle++;
  

  write_result();
  printf("Terminou write_result\n");
  execute();
  printf("Terminou execute\n");
  read_operands();
  printf("Terminou read_operands\n");
  issue();
  printf("Terminou issue_instruction\n");
  fetch();
  printf("Terminou fetch\n");


  for (int i = 0; i < g_total_ufs; i++){
    printf("Printando uf de index %d\n", i);
    print_uf(g_functional_units[i]);
  }

  // Imprimindo tabelas, não fiel à simulação
  Byte inst_opcodes[g_instruction_count];
  for (int i = 0; i < g_instruction_count; i++){
    int inst = get_instruction_from_memory(i, g_memory);
    int opcode = get_opcode_from_binary(inst);
    inst_opcodes[i] = opcode;
    printf("inst %d: %d\n", i, inst_opcodes[i]);
  }
  print_table(&g_score_board, g_current_cycle, inst_opcodes, g_instruction_count, g_total_ufs);
  print_registers(g_registers);
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



// Main e leitura da entrada

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
    g_memory = (Byte*)(malloc(sizeof(Byte)* (PROGRAM_FIRST_ADDRESS + g_memory_size*4)));

    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);

    if (parse_assembly(input_file, output_stream, &g_cpu_configs, &g_instruction_count, g_memory, g_memory_size)){
      malloc_memory(&g_functional_units, &g_score_board, g_cpu_configs, g_instruction_count, g_memory_size);
      init_scoreboard(&g_score_board);
      init_functional_units(g_functional_units, g_cpu_configs);
      g_total_ufs = g_cpu_configs.size_add_ufs + g_cpu_configs.size_integer_ufs + g_cpu_configs.size_mul_ufs;

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

  // free_memory(input_file, output_stream, &g_score_board, &g_functional_units);

  return 0;

}
