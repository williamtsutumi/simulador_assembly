#ifndef TABLE_H
#define TABLE_H

#define NUM_COLS_INSTRUCTION_STATUS 6
#define NUM_COLS_FUNCTIONAL_UNIT_STATUS 10
#define NUM_COLS_RESULT_REGISTER_STATUS 32
#define NUM_COLS_REGISTER_RESULT 32

#include "stdio.h"

typedef enum TABLE_CELL_TYPE{
    T_UF_NAME,
    T_BOOL,
    T_OP,
    T_REGISTER_READ,
    T_REGISTER_WRITE,
    T_INSTRUCTION,
    T_NUM,
    T_EXECUTION,
} TABLE_CELL_TYPE;

typedef enum TABLE_TYPE{
    INSTRUCTION_STATUS,
    FUNCTIONAL_UNIT_STATUS,
    RESULT_REGISTER_STATUS,
    REGISTER_RESULT
} TABLE_TYPE;

typedef struct Table{
    int num_columns, num_rows;
    char*** data;
    char** headers;
    char* title;
    TABLE_TYPE type;

} Table;

void table_init(Table *table, TABLE_TYPE type);
void table_print(Table* table, FILE* output);
void add_row(Table* table, ...);
void free_table(Table* table);

#endif
