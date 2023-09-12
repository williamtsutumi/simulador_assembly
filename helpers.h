#ifndef HELPERS
#define HELPERS

#include "types.h"

/* DEBUG helpers */

void print_ufs_current_cycle(FILE *output, CPU_Configurations cpu_configs, FunctionalUnit functional_units[]){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (i == 0) printf("ADD UFS:\n");
    else if (i == cpu_configs.size_add_ufs) printf("MUL UFS:\n");
    else if (i == cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs) printf("INTEGER UFS:\n");
    printf("functional unit [%d].cycle: %d\n", i, functional_units[i].current_cycle);
  }
}



/* fetch_next_instruction helpers */

/* issue_instruction helpers */

bool has_idle_uf(){

}

FunctionalUnitState *find_uf_with_type(UF_TYPE type){

}

FunctionalUnitState get_instruction_information(int instruction_binary){

}

/* read_operands helpers */

/* execute helpers */

/* write_result helpers */

/* Outros */

bool uf_is_ready(FunctionalUnit uf, CPU_Configurations cpu_configs){
  if (uf.type == ADD_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_add;
  else if (uf.type == MUL_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_mul;
  else if (uf.type == INTEGER_UF)
    return uf.current_cycle == cpu_configs.cycles_to_complete_integer;
  return false;
}

void increment_all_uf_current_cycle(FILE *output, CPU_Configurations cpu_configs, FunctionalUnit functional_units[]){
  int total_ufs = cpu_configs.size_add_ufs + cpu_configs.size_mul_ufs + cpu_configs.size_integer_ufs;
  for (int i=0; i<total_ufs; i++){
    if (uf_is_ready(functional_units[i], cpu_configs)){
      functional_units[i].current_cycle = 1;
      // write_result(functional_units[i]);
      continue;
    }
    // if is busy
    functional_units[i].current_cycle++;
  }
  print_ufs_current_cycle(output, cpu_configs, functional_units);
}

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


#endif