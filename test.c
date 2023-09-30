#include <stdio.h>
#include "headers/table.h"

int main(){

  Table t1;
  table_init(&t1, INSTRUCTION_STATUS);
  add_row(&t1, 1, 3, 2, 3, 4, 5, 6);
  table_print(&t1);
  free_table(&t1);
  Table t2;
  table_init(&t2, FUNCTIONAL_UNIT_STATUS);
  add_row(&t2, 1, 0, 3, 4, 5, 6, 1, 2, 0, 0, 1, 2, 1);
  add_row(&t2, 1, 1, 3, 3, 0, 1, 2, 3, 1, 1, 1, 2, 0);
  table_print(&t2);
  free_table(&t2);
  Table t3;
  int r[32] = {0};
  r[5] = -1;
  table_init(&t3, REGISTER_RESULT);
  add_row(&t3, r);
  table_print(&t3);
  free_table(&t3);


}


