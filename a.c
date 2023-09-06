#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_NUM_ROWS_TABLE 100

#include "assembly_parser.h"
#include "types.h"

CPU_Configurations cpu_configs;
FunctionalUnit *functional_units;

int *memory;
int *registrers;
const char *code_file_name;
int *instructions;
int instruction_count = 0;

int global_current_cycle = 0;
int program_counter;

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
  free(functional_units);
  free(instructions);

  fclose(input);
  fclose(output);
}

void write_result(){

}

void print_ufs_current_cycle(FILE *output){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (i == 0) printf("ADD UFS:\n");
    else if (i == cpu_configs.size_add_ufs) printf("MUL UFS:\n");
    else if (i == cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs) printf("INTEGER UFS:\n");
    printf("functional unit [%d].cycle: %d\n", i, functional_units[i].current_cycle);
  }
}

bool uf_is_ready(FunctionalUnit uf){
  if (uf.type == ADD_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_add;
  else if (uf.type == MUL_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_mul;
  else if (uf.type == INTEGER_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_integer;
  return false;
}

void increment_all_uf_current_cycle(FILE *output){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (uf_is_ready(functional_units[i])){
      functional_units[i].current_cycle = 1;
      // write_result(functional_units[i]);
      continue;
    }
    // if is busy
    functional_units[i].current_cycle++;
  }
  print_ufs_current_cycle(output);
}

void run_one_cycle(FILE *output){
  fprintf(output, "Faz alguma coisa hamada\n");
  global_current_cycle++;
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

void malloc_cpu(){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  functional_units = (FunctionalUnit*)malloc(total_ufs * sizeof(FunctionalUnit));
  for (int i=0; i<total_ufs; i++){
    if (i < cpu_configs.size_add_ufs)
      functional_units[i].type = ADD_UF;
    else if (i < cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs)
      functional_units[i].type = MUL_UF;
    else
      functional_units[i].type = INTEGER_UF;
  }
}

int main(int argc, char *argv[])
{
  code_file_name = argv[0];

  __table_tests();

  int memory_size = 32;
  FILE* output_stream = stdout;
  char* input_file_name = "input.sb";
  FILE* input_file;
  
  if (read_args(argc, argv, &memory_size, &input_file_name, &input_file, &output_stream)){
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);
    if (parse_assembly(input_file, output_stream, &cpu_configs, &instructions)){
      malloc_cpu();
      run_simulation(output_stream);
    }
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  free_memory(input_file, output_stream);

  return 0;

}
