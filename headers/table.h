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
    T_REGISTER_READ,
    T_REGISTER_WRITE,
    T_INSTRUCTION,
    T_NUM,
    T_UF_NAME_POINTER,
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

void table_init(Table *table, TABLE_TYPE type){

    table->num_rows = 0;
    table->num_columns = 0;
    table->data = NULL;
    table->headers = NULL;
    table->title = NULL;

    char* title;
    switch(type){

        case INSTRUCTION_STATUS:
            table->num_columns = NUM_COLS_INSTRUCTION_STATUS;
            table->type = INSTRUCTION_STATUS;
            title = "Status das Instruções";
            table->title = (char*)malloc(sizeof(char)*strlen(title)+1);
            strcpy(table->title, title);
            
            
            
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
            title = "Status das Unidades Funcionais";
            table->title = (char*)malloc(sizeof(char)*strlen(title)+1);
            strcpy(table->title, title);

            table->headers = (char**)malloc(sizeof(char*) * NUM_COLS_FUNCTIONAL_UNIT_STATUS);
            

            for(int i = 0; i < NUM_COLS_FUNCTIONAL_UNIT_STATUS; i++){
                table->headers[i] = (char*)malloc(strlen(uf_header[i]) + 1);
                strcpy(table->headers[i], uf_header[i]); 
            }

            break;

        case RESULT_REGISTER_STATUS:
            table->num_columns = NUM_COLS_RESULT_REGISTER_STATUS;
            table->type = RESULT_REGISTER_STATUS;
            title = "Status do Resultado dos Registradores";
            table->title = (char*)malloc(sizeof(char)*strlen(title)+1);
            strcpy(table->title, title);


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
            title = "Resultado dos Registradores";
            table->title = (char*)malloc(sizeof(char)*strlen(title)+1);
            strcpy(table->title, title);

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

static void format_uf_name(char* result, UF_TYPE uf_type, int uf_index){
    switch(uf_type){
        case INTEGER_UF:
            sprintf(result, "Integer%d", uf_index);
            break;
        case MUL_UF:
            sprintf(result, "Mul%d", uf_index);
            break;
        case ADD_UF:
            sprintf(result, "Add%d", uf_index);
            break;
        case NOT_APPLIED_UF:
            sprintf(result, "Null%d", uf_index);
            break;
    }
}

static void format_execution(char* result, int start_time, int end_time){
    if(start_time == -1){
        result[0] = '\0';
    }
    else{
        sprintf(result, "%d - %d", start_time, end_time);
    }
}

static void format_bool(char* result, int num){
    if(num == -1){
        result[0] = '\0';
    }
    else if(num){
        sprintf(result, "Yes");
    }
    else{
        sprintf(result, "No");
    }
}

static void format_op(char* result, int opcode){
    if(opcode == -1){
        result[0] = '\0';
    }
    else{
        char* instruction_names[] = INSTRUCTION_NAMES;
        sprintf(result, "%s", instruction_names[opcode]);
    }
}

static void format_register_write(char* result, int register_idx){
    if(register_idx == -1){
        result[0] = '\0';
    }
    else if(register_idx == 0){
        sprintf(result, "PC");
    }
    else{
        sprintf(result, "R%d", register_idx);
    }

}
static void format_register_read(char* result, int register_idx){
    if(register_idx != -1){
        sprintf(result, "R%d", register_idx);
    }
    else{
        result[0] = '\0';
    }
}

static void format_num(char* result, int num){
    if(num != -1){
        sprintf(result, "%d", num);
    }
    else{
        result[0] = '\0';
    }
    
}

static void format_instruction(char* result, InstructionBinary instruction){
    // temporario
    int op = instruction >> 26;

    char* instruction_names[] = INSTRUCTION_NAMES;
    sprintf(result, "%s", instruction_names[op]);
}



void add_row(Table* table, ...){
    va_list args;
    va_start(args, table);


    table->num_rows++;

    // INSTRUCTION_STATUS,
    // FUNCTIONAL_UNIT_STATUS,
    // RESULT_REGISTER_STATUS,
    // REGISTER_RESULT
    int expected_cell_type[][10] = {
        {T_INSTRUCTION, T_NUM,  T_NUM, T_NUM,      T_EXECUTION,      T_NUM,      -1,        -1,        -1,     -1},
        {T_UF_NAME,     T_BOOL, T_OP,  T_REGISTER_WRITE, T_REGISTER_READ, T_REGISTER_READ, T_UF_NAME_POINTER, T_UF_NAME_POINTER, T_BOOL, T_BOOL},     
    };

    table->data = (char***)realloc(table->data, sizeof(char**) * (table->num_rows));
    table->data[table->num_rows - 1] = (char**)malloc(sizeof(char*) * (table->num_columns));
 
    char result[64];
    
    if(table->type == INSTRUCTION_STATUS || table->type == FUNCTIONAL_UNIT_STATUS){
        
        for(int i = 0; i < table->num_columns; i++){
            switch(expected_cell_type[table->type][i]){
                case T_UF_NAME:
                    UF_TYPE uf_type = va_arg(args, UF_TYPE);
                    int uf_index = va_arg(args, int);
                    
                    format_uf_name(result, uf_type, uf_index);

                    break;
                case T_BOOL:
                    int boolean = va_arg(args, int);

                    format_bool(result, boolean);
                    break;
                case T_OP:
                    int opcode = va_arg(args, int);

                    format_op(result, opcode);
                    break;
                case T_REGISTER_WRITE:
                    int register_idx = va_arg(args, int);

                    format_register_write(result, register_idx);
                    break;
                case T_REGISTER_READ:
                    int register_idxx = va_arg(args, int);

                    format_register_read(result, register_idxx);
                    break;
                case T_NUM:
                    int num = va_arg(args, int);

                    format_num(result, num);
                    break;
                case T_INSTRUCTION:
                    int instruction = va_arg(args, InstructionBinary);

                    format_instruction(result, instruction);
                    break;
                case T_UF_NAME_POINTER:
                    FunctionalUnit* uf = va_arg(args, FunctionalUnit*);
                    if(uf == NULL){
                        format_num(result, -1);
                    }
                    else{
                        format_uf_name(result, uf->type, uf->type_index);
                    }
                    break;
                case T_EXECUTION:
                    int start_time = va_arg(args, int);
                    int end_time = va_arg(args, int);

                    format_uf_name(result, start_time, end_time);
                case -1:
                    break;
            }
            table->data[table->num_rows-1][i] = (char*)malloc(sizeof(char)*strlen(result)+1);
            strcpy(table->data[table->num_rows-1][i], result);
        }
        va_end(args);
    }
    else if(table->type == REGISTER_RESULT){
        int* register_info = va_arg(args, int*);
        for(int i = 0; i < NUM_REGISTERS; i++){
            format_num(result, register_info[i]);
            table->data[table->num_rows-1][i] = (char*)malloc(sizeof(char)*strlen(result)+1);
            strcpy(table->data[table->num_rows-1][i], result);
        }
    }
    else if(table->type == RESULT_REGISTER_STATUS){ 
        FunctionalUnit** result_register_state = va_arg(args, FunctionalUnit**);
        
        for(int i = 0; i < NUM_REGISTERS; i++){
            if(result_register_state[i] == NULL){
                format_num(result, -1);
            }
            else{
                format_uf_name(result, result_register_state[i]->type, result_register_state[i]->type_index);
            }
            table->data[table->num_rows-1][i] = (char*)malloc(sizeof(char)*strlen(result)+1);
            strcpy(table->data[table->num_rows-1][i], result);
        }
    }
}



// [starting_column, end_column)
static void print_headers(Table *table, int starting_column, int end_column){
    yellow();
    printf("|");
    for(int i = starting_column; i < end_column; i++){
        if(table->type == RESULT_REGISTER_STATUS || table->type == REGISTER_RESULT)
            printf("%-8s|", table->headers[i]);
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
            printf("%-8s|", table->data[i][j]);
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

static void print_title(Table *table){
    blue_background();
    printf("%s", table->title);
    reset_background();
    printf("\n");
    
    
}

void table_print(Table* table){
    print_title(table);
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
        for(int j = 0; j < table->num_columns; j++){
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
    free(table->title);
}


#endif
