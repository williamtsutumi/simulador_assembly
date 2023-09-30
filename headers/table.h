#ifndef TABLE_H
#define TABLE_H

#include <stdarg.h>
#include "types.h"
#include "assembly_parser.h" // para ter yellow(). Tem q tirar de lá

#define NUM_COLS_INSTRUCTION_STATUS 6
#define NUM_COLS_FUNCTIONAL_UNIT_STATUS 10
#define NUM_COLS_RESULT_REGISTER_STATUS 32
#define NUM_COLS_REGISTER_RESULT 32

typedef enum TABLE_CELL_TYPE{
    T_UF_NAME,
    T_BOOL,
    T_OP,
    T_REGISTER,
    T_INSTRUCTION,
    T_NUM
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
    TABLE_TYPE type;

} Table;

void table_init(Table *table, TABLE_TYPE type){

    table->num_rows = 0;

    switch(type){

        case INSTRUCTION_STATUS:
            table->num_columns = NUM_COLS_INSTRUCTION_STATUS;
            table->type = INSTRUCTION_STATUS;
            
            
            char* instruction_header[] = {"Instruction", "Fetch", "Issue", "Read operands", "Execution", "Write result" };

            table->headers = (char**)malloc(sizeof(char*) * NUM_COLS_INSTRUCTION_STATUS);
            

            for (int i = 0; i < NUM_COLS_INSTRUCTION_STATUS; i++) {
                table->headers[i] = (char*)malloc(strlen(instruction_header[i]) + 1);
                strcpy(table->headers[i], instruction_header[i]); 
            }

            break;
        case FUNCTIONAL_UNIT_STATUS:
            table->num_columns = NUM_COLS_FUNCTIONAL_UNIT_STATUS;
            table->type = FUNCTIONAL_UNIT_STATUS;
            char* uf_header[] = {"Name", "Busy", "Op", "Fi", "Fj", "Fk", "Qj", "Qk", "Rj", "Rk"};

            table->headers = (char**)malloc(sizeof(char*) * NUM_COLS_FUNCTIONAL_UNIT_STATUS);
            

            for (int i = 0; i < NUM_COLS_FUNCTIONAL_UNIT_STATUS; i++) {
                table->headers[i] = (char*)malloc(strlen(uf_header[i]) + 1);
                strcpy(table->headers[i], uf_header[i]); 
            }

            break;

        case RESULT_REGISTER_STATUS:
            table->num_columns = NUM_COLS_RESULT_REGISTER_STATUS;
            table->type = RESULT_REGISTER_STATUS;


            table->headers = (char**)malloc(sizeof(char*) * NUM_COLS_RESULT_REGISTER_STATUS);
            

            for(int i = 0; i < NUM_COLS_RESULT_REGISTER_STATUS; i++){
                char register_header[10];
                sprintf(register_header, "R%d", i);
                table->headers[i] = (char*)malloc(strlen(register_header)+1);
                strcpy(table->headers[i], register_header);
            }
            break;

        case REGISTER_RESULT:
            table->num_columns = NUM_COLS_REGISTER_RESULT;
            table->type = REGISTER_RESULT;

            table->headers = (char**)malloc(sizeof(char*) * NUM_COLS_REGISTER_RESULT);
            

            for(int i = 0; i < NUM_COLS_REGISTER_RESULT; i++){
                char local_header[10];
                sprintf(local_header, "R%d", i);
                table->headers[i] = (char*)malloc(strlen(local_header)+1);
                strcpy(table->headers[i], local_header);
            }
            break;

    }
}

static char* format_uf_name(int uf_type, int uf_index){
    char* 
}

static char* format_bool(int num){

}

static char* format_op(int opcode){

}

static char* format_register(int register_idx){

}

static char* format_num(int num){

}

static char* format_instruction(InstructionBinary instructio){

}



void add_row(Table* table, ...){
    va_list args;
    va_start(args, table);


    table->num_rows++;
    table->data = (char***)realloc(table->data, (sizeof(char**)*table->num_columns));

    // INSTRUCTION_STATUS,
    // FUNCTIONAL_UNIT_STATUS,
    // RESULT_REGISTER_STATUS,
    // REGISTER_RESULT
    int expected_cell_type[][] = {
        {T_INSTRUCTION, T_NUM,  T_NUM, T_NUM,      T_NUM,      T_NUM,      -1,        -1,        -1,     -1},
        {T_UF_NAME,     T_BOOL, T_OP,  T_REGISTER, T_REGISTER, T_REGISTER, T_UF_NAME, T_UF_NAME, T_BOOL, T_BOOL},     
    };

    if(table->type == INSTRUCTION_STATUS || table->type == FUNCTIONAL_UNIT_STATUS){
        char buffer[64];
        char* result;
        for(int i = 0; i < table->num_columns; i++){
            switch(expected_cell_type[table->type][i]){
                case T_UF_NAME:
                    int uf_type = va_arg(args, int);
                    int uf_index = va_arg(args, int);

                    result = format_uf_name(uf_type, uf_index);
                    break;
                case T_BOOL:
                    int boolean = va_arg(args, int);

                    result = format_bool(boolean);
                    break;
                case T_OP:
                    int opcode = va_arg(args, int);

                    result = format_op(opcode);
                    break;
                case T_REGISTER:
                    int register_idx = va_arg(args, int);

                    result = format_register(register_idx);
                    break;
                case T_NUM:
                    int num = va_arg(args, int);

                    result = format_num(opcode);
                    break;
                case T_INSTRUCTION:
                    int instruction = va_arg(args, InstructionBinary);

                    result = format_instruction(instruction);
                    break;
            }
            strcpy(table->data[table->num_rows-1][i], result);
        }
        va_end(args);
        free(buffer);
    }
    else{
        int* register_info = va_arg(args, int*);


    }
}



// [starting_column, end_column)
static void print_headers(Table *table, int starting_column, int end_column){
    yellow();
    printf("|");
    for(int i = starting_column; i < end_column; i++){
        if(table->type == RESULT_REGISTER_STATUS || table->type == REGISTER_RESULT)
            printf("%-5s|", table->headers[i]);
        else
            printf("%-13s|", table->headers[i]);

    }
    printf("\n");
    reset();
}

static void print_data(Table *table, int starting_column, int end_column){
    for(int i = 0; i < table->num_rows; i++){
        printf("|");
        for(int j = starting_column; j < end_column; j++){
            if(table->type == RESULT_REGISTER_STATUS || table->type == REGISTER_RESULT)
            printf("%-5s|", table->data[i][j]);
        else
            printf("%-13s|", table->data[i][j]);
        }
        printf("\n");
    }
}

// [starting_column, end_column)
static void __print_table(Table* table, int starting_column, int end_column){
    print_headers(table, starting_column, end_column);
    print_data(table, starting_column, end_column);
}

void print_table(Table* table){
    if(table->num_columns > 16){
        __print_table(table, 0, 16);
        __print_table(table, 16, table->num_columns);
    }
    else{
        __print_table(table, 0, table->num_columns);
    }
}

void free_table(Table* table){
    

    for(int i = 0; i < table->num_rows; i++){
        for(int j = 0; j < table->num_columns; i++){
            free(table->data[i][j]);
        }
    }

    for (int i = 0; i < table->num_rows; i++){
        free(table->data[i]);
    }

    for(int i = 0; i < table->num_columns; i++){
        free(table->headers[i]);
    }

    free(table->data);
    free(table->headers);
}


#endif
