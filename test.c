#include <stdio.h>
#include "headers/table.h"

int main(){

  Table t1;
  table_init(&t1, INSTRUCTION_STATUS);
  Table t2;
  table_init(&t2, FUNCTIONAL_UNIT_STATUS);
  Table t3;
  table_init(&t3, RESULT_REGISTER_STATUS);

  print_table(&t1);
  print_table(&t2);
  print_table(&t3);

}


