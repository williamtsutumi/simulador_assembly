#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#include "assembly_parse.h"

#define MAX_NUM_ROWS_TABLE 100

#include "types.h"

void init_table(Table* table, TableElementType type) {
    table->type = type;
    table->num_rows = 0;
    table->table = (Table_entry*)malloc(MAX_NUM_ROWS_TABLE * sizeof(Table_entry));
}

void insert_in_table(Table* table, void* item_ptr) {
    TableElementType type = table->type;

    if (table->num_rows < MAX_NUM_ROWS_TABLE) {
        Table_entry *table_entry = &table->table[table->num_rows++];

        switch (type) {
            case FUNCTIONAL_UNIT:
                table_entry->data.functional_unit = (struct FunctionalUnit*)item_ptr;
                break;
            case INSTRUCTION:
                table_entry->data.instruction = (struct Instruction*)item_ptr;
                break;
            case REGISTER_INFO:
                table_entry->data.register_info = (struct RegisterInfo*)item_ptr;
                break;
            default:
                break;
        }
    } else {
        // Handle table full error
    }
}

void print_table(Table* table){
  for(int i = 0; i < table->num_rows; i++){
    printf("%d\n", table->table->data.functional_unit->current_cycle);
  }
}



CPU cpu;
int* memory;
int instruction_count = 0;
const char* code_file_name;
int *instructions;

int add_cycles, mul_cycles, integer_cycles;

int dec_to_bin(int num){

  for(int i = 0; i < 32; i++){
    printf("%d", (num&(1 << (31-i)))!=0);
  }
  putchar('\n');

}

// verifica se na posição SEEK_CUR do arq, existe uma string do tipo 
// .secao, onde secao é o nome da seção, se o nome é .data, retorna 0,
// se for .text, retorna 1, se não tiver seção, retorna -1;
// 
int find_section(FILE *arq){

  skip(arq);
  return -1;
}

bool read_args(int argc, char *argv[], int *memory_size, char **input_file_name, FILE **input_file, FILE **output_stream){
  for(int i = 1; i < argc; i+=2){

    if(strcmp(argv[i], "-p") == 0){
      *input_file_name = argv[i+1];
    }
    else if(strcmp(argv[i], "-m") == 0){
      // TODO
      // validar que argv[i+1] é uma inteiro
      *memory_size = atoi(argv[i+1]);
      memory = (int*)(malloc(sizeof(int)*(*memory_size)));
    }
    else if(strcmp(argv[i], "-o") == 0){
      *output_stream = fopen(argv[i+1], "w");
    }
  }

  *input_file = fopen(*input_file_name, "r");
  return *input_file != NULL;
}

void free_memory(FILE *input, FILE*output){
  free(cpu.add_ufs);
  free(cpu.mul_ufs);
  free(cpu.integer_ufs);
  free(instructions);

  fclose(input);
  fclose(output);
}

void write_result(){

}

void print_ufs_current_cycle(FILE *output){
  fprintf(output, "add ufs: \n");
  for(int i=0; i<cpu.size_add_ufs; i++)
    fprintf(output, "%d\n", cpu.add_ufs[0].current_cycle);

  fprintf(output, "mul ufs: \n");    
  for(int i=0; i<cpu.size_mul_ufs; i++)
    fprintf(output, "%d\n", cpu.mul_ufs[0].current_cycle);

  fprintf(output, "integer ufs: \n");
  for(int i=0; i<cpu.size_integer_ufs; i++)
    fprintf(output, "%d\n", cpu.integer_ufs[0].current_cycle);
}

void increment_all_uf_current_cycle(FILE *output){
  for (int i=0; i<cpu.size_add_ufs; i++){
    if (cpu.add_ufs[i].current_cycle == add_cycles){
      cpu.add_ufs[i].current_cycle = 1;
      write_result();
      continue;
    }
    // if is executing instruction
    cpu.add_ufs[i].current_cycle++;
  }
  for (int i=0; i<cpu.size_mul_ufs; i++){
    if (cpu.mul_ufs[i].current_cycle == mul_cycles){
      cpu.mul_ufs[i].current_cycle = 1;
      write_result();
      continue;
    }
    // if is executing instruction
    cpu.mul_ufs[i].current_cycle++;
  }
  for (int i=0; i<cpu.size_integer_ufs; i++){
    if (cpu.integer_ufs[i].current_cycle == integer_cycles){
      cpu.integer_ufs[i].current_cycle = 1;
      write_result();
      continue;
    }
    // if is executing instruction
    cpu.integer_ufs[i].current_cycle++;
  }
  print_ufs_current_cycle(output);
}

void run_one_cycle(FILE *output){
  fprintf(output, "Faz alguma coisa hamada\n");
  increment_all_uf_current_cycle(output);
}

void run_simulation(FILE *output){
  bool has_active_instruction = true;
  while (has_active_instruction){
    getchar();
    run_one_cycle(output);
  }
}

void __table_tests(){
  Table tabela;
  init_table(&tabela, FUNCTIONAL_UNIT);

  FunctionalUnit tb;
  tb.current_cycle = 1;

  insert_in_table(&tabela, &tb);

  print_table(&tabela);
  tb.current_cycle = 3;

  print_table(&tabela);

}

int main(int argc, char *argv[])
{
  code_file_name = argv[0];
  // FunctionalUnit add;
  // FunctionalUnit mul;
  // FunctionalUnit integer;

  __table_tests();

  int memory_size = 32;
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &memory_size, &input_file_name, &input_file, &output_stream)){
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);
    if (parse_assembly(input_file, output_stream, &cpu)){
      printf("AAA\n");
      printf("%li\n", sizeof(cpu.add_ufs) / sizeof(cpu.add_ufs[0]));
      printf("%li\n", sizeof(cpu.mul_ufs) / sizeof(cpu.mul_ufs[0]));
      printf("%li\n", sizeof(cpu.integer_ufs) / sizeof(cpu.integer_ufs[0]));
      printf("AAA\n");
      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream);

  return 0;

}
