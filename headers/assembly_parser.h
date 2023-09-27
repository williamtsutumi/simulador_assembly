#ifndef ASSEMBLY_PARSER
#define ASSEMBLY_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"

//****************Declarações das funções**************//

void red();
void yellow();
void reset();
void print_str_int(char *, FILE *);
void print_str_char(char *, FILE *);

int read_number(FILE *, bool);
bool validate_number(char *);
void decapitalize(char *);
void skip(FILE *);
void die(FILE *, char *);
void fpeek(FILE *, char *, int);
bool read_next_token(FILE *, char *, bool);

int read_instructionR(int, int, int, int, int);
int read_instructionI(int, int, int, int);
int read_instructionJ(int, int);

int get_opcode(char *);
int read_instruction_given_opcode(int, FILE *);
int read_operand(FILE *, OPERAND_TYPE, bool);
int read_register_id(FILE *);
int read_instruction_name(FILE *, FILE *);
int read_instruction(FILE *, FILE *);

bool read_uf(FILE *, FILE *, CPU_Configurations *);
bool read_inst(FILE *, FILE *, CPU_Configurations *);
bool read_config(FILE *, FILE *, CPU_Configurations *);

bool read_data_section(FILE *, Byte **, int);

bool parse_assembly(FILE *, FILE *, CPU_Configurations *, int *, Byte **, int);

//*****************************************************//

int read_instructionR(int opcode, int rd, int rs, int rt, int extra)
{
    int op = opcode << 26;
    int s = rs << 21;
    int t = rt << 16;
    int d = rd << 11;
    int e = extra << 0;

    return op | s | t | d | e;
}
int read_instructionI(int opcode, int rs, int rt, int imm)
{
    int op = opcode << 26;
    int s = rs << 21;
    int r = rt << 16;

    return op | s | r | imm;
}
int read_instructionJ(int opcode, int address)
{
    int op = opcode << 26;

    return op | address;
}

void red() {
  printf("\033[1;31m");
}
void yellow() {
  printf("\033[1;33m");
}
void reset() {
  printf("\033[0m");
}
void print_str_int(char *string, FILE *output){
  fprintf(output, "String:");
  for(int i=0; i<strlen(string)-1; i++){
    fprintf(output, "%d ", string[i]);
  }
  fprintf(output, "%d", string[strlen(string)-1]);
  fprintf(output, ":Fim da string\n");
}
void print_str_char(char *string, FILE *output){
  fprintf(output, "String:");
  for(int i=0; i<strlen(string)-1; i++){
    fprintf(output, "%c", string[i]);
  }
  fprintf(output, "%c", string[strlen(string)-1]);
  fprintf(output, ":Fim da string\n");
}

void fpeek(FILE* arq, char* peekBuffer, int peekSize){
  char character;
  int i=0;
  fseek(arq, -1, SEEK_CUR);
  for(i=0; i < peekSize; i++){
    if((character = fgetc(arq)) != EOF){
      peekBuffer[i] = character;
    }
    else{
      break;
    }
  }
  peekBuffer[i] = '\0';
  fseek(arq, -i+1, SEEK_CUR);
}

void decapitalize(char *str){
  int len = strlen(str);
  for(int i = 0; i < len; i++){
    if('A' <= str[i] && str[i] <= 'Z') str[i] -= 'A';
  }
}

int get_opcode(char* str){

  // puts(str);
  if(strncmp(ADDI, str, strlen(ADDI)) == 0) return ADDI_OPCODE;
  if(strncmp(SUBI, str, strlen(SUBI)) == 0) return SUBI_OPCODE;
  if(strncmp(EXIT, str, strlen(EXIT)) == 0) return EXIT_OPCODE;
  if(strncmp(ADD, str, strlen(ADD)) == 0) return ADD_OPCODE;
  if(strncmp(SUB, str, strlen(SUB)) == 0) return SUB_OPCODE;
  if(strncmp(MUL, str, strlen(MUL)) == 0) return MUL_OPCODE;
  if(strncmp(DIV, str, strlen(DIV)) == 0) return DIV_OPCODE;
  if(strncmp(AND, str, strlen(AND)) == 0) return AND_OPCODE;
  if(strncmp(NOT, str, strlen(NOT)) == 0) return NOT_OPCODE;
  if(strncmp(BLT, str, strlen(BLT)) == 0) return BLT_OPCODE;
  if(strncmp(BGT, str, strlen(BGT)) == 0) return BGT_OPCODE;
  if(strncmp(BEQ, str, strlen(BEQ)) == 0) return BEQ_OPCODE;
  if(strncmp(BNE, str, strlen(BNE)) == 0) return BNE_OPCODE;
  if(strncmp(OR, str, strlen(OR)) == 0) return OR_OPCODE;
  if(strncmp(LW, str, strlen(LW)) == 0) return LW_OPCODE;
  if(strncmp(SW, str, strlen(SW)) == 0) return SW_OPCODE;
  if(strncmp(J, str, strlen(J)) == 0) return J_OPCODE;
  else return -1;
  

}

int read_instruction_given_opcode(int opcode, FILE* arq){

  int rd=0, rs=0, rt=0, imm=0, extra=0;

  switch(opcode){

    // tratar erros
    case ADD_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case ADDI_OPCODE:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case SUB_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case SUBI_OPCODE:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case MUL_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);

      return read_instructionR(opcode, rd, rs, rt, 0);

    case DIV_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case AND_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case OR_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case NOT_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rd, rs, rt, 0);

    case BLT_OPCODE:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case BGT_OPCODE:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case BEQ_OPCODE:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case BNE_OPCODE:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case J_OPCODE:
      imm = read_operand(arq, IMM, false);
      return read_instructionJ(opcode, imm);

    case LW_OPCODE:
      rt = read_operand(arq, REGISTER, true);

      imm = read_operand(arq, IMM, false);
      if (!read_next_token(arq, "(", false)) break;
      if (!read_next_token(arq, "r", false)) break; 
      rs = read_number(arq, true);
      if (!read_next_token(arq, ")", false)) break;

      return read_instructionI(opcode, rs, rt, imm);

    case SW_OPCODE:
      rt = read_operand(arq, REGISTER, true);

      imm = read_operand(arq, IMM, false);
      if (!read_next_token(arq, "(", false)) break;
      if (!read_next_token(arq, "r", false)) break;
      rs = read_number(arq, true);
      if (!read_next_token(arq, ")", false)) break;

      return read_instructionI(opcode, rs, rt, imm);

    case EXIT_OPCODE:
      return read_instructionJ(opcode, 1);
  }

  assert(false);
  return -1;

}

// Lê as informações da seção .data, que está no SEEK_CUR
bool read_data_section(FILE *arq, Byte **memory, int memory_size){
  if (!read_next_token(arq, ".data", true)){
    return false;
  }
  for (int i = 0; i < memory_size; i += 4){
    char c = getc(arq);
    while (isspace(c)) c = fgetc(arq);

    fseek(arq, -1, SEEK_CUR);
    if (isdigit(c)){
      int num = read_number(arq, true);
      (*memory)[i + 3] = (num >> 24) & 0b11111111;
      (*memory)[i + 2] = (num >> 16) & 0b11111111;
      (*memory)[i + 1] = (num >> 8) & 0b11111111;
      (*memory)[i] = (num >> 0) & 0b11111111;
      // printf("%d\n", (*memory)[i] | (*memory)[i + 1] << 8 | (*memory)[i + 2] << 16 | (*memory)[i + 3] << 24);
    }
  }
  return true;
}

int read_instruction(FILE *arq, FILE *output){
  char *expected_tokens[] = INSTRUCTION_NAMES;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);

  for (int opcode=0; opcode < num_tokens; opcode++){
    if(read_next_token(arq, expected_tokens[opcode], true)){
      printf("LEU %s, opcode: %d\n", expected_tokens[opcode], opcode);
      return read_instruction_given_opcode(opcode, arq);
    }
  }

  return -1;

}

// retorna um inteiro que representa um operando do tipo type
// a partir de SEEK_CUR, se não exister nenhum, retorna -1;
int read_operand(FILE* arq, OPERAND_TYPE type, bool expect_comma){
  
  if(type == REGISTER){
    if(!read_next_token(arq, "r", true)) return -1;

    int register_id = read_number(arq, false);
    if(expect_comma && !read_next_token(arq, ",", true)){
      die(arq, "Expected ','");
    }

    return register_id;
  }
  else if(type == IMM){
    return read_number(arq, true);
  }
  else if(type == MEMORY){
    int desvio = read_number(arq, true);
    if(!read_next_token(arq, "(", true))
      die(arq, "Expected '('");
    if(!read_next_token(arq, "r", true))
      die(arq, "Expected 'r'");
    int register_id = read_number(arq, false);

    if(!read_next_token(arq, ")", true))
      die(arq, "Expected ')'");

    return -1;
  }

  // pula espaço em branco no fim da execução
  skip(arq);

  return 0;
}

// verifica se a partir de SEEK_CUR, existe uma instrução
// por exemplo, "add rs,rs,rt" e retorna apenas seu opcode.
// se não existe nenhum, retorna -1
int read_instruction_name(FILE *input, FILE *output){
  // printf("ftell: %li", ftell(input));
  char buffer[10];
  char c;
  int cnt=0;

  while((c = fgetc(input)) != EOF){
    if(c == ' '){
      break;
    }
    buffer[cnt++] = c;
  }
  buffer[cnt] = '\0';

  int opcode = get_opcode(buffer);

  skip(input);
  return opcode;
}

int read_register_id(FILE *arq){
  char reg_start = fgetc(arq);

  if(reg_start != 'r'){
    die(arq, "Unexpected symbol");

  }

  char buffer[10];
  char c;
  int cnt=0;

  while((c = fgetc(arq)) != EOF){
    if(c == ' ' || c == ','){
      if(c == ',')
        fseek(arq, -1, SEEK_CUR);
      break;
    }
    buffer[cnt++] = c;
  }
  buffer[cnt] = '\0';

  if(!validate_number(buffer)){
    die(arq, "invalid register id");
  }
  return atoi(buffer);

}

bool validate_number(char *c){
  return true;
}

// le espaços brancos, tabs, comentários, etc até que encontre alguma 
// coisa interessante;
void skip(FILE *arq){
  char c;
  
  bool on_single_line_comment = false;

  while((c = fgetc(arq)) != EOF){
    if(c == '#'){
      on_single_line_comment = true;
    }
    else if(on_single_line_comment && c == '\n'){
      on_single_line_comment = false;
    }
    else if(on_single_line_comment || c == ' ' || c == '\t' || c == '\n'){
      continue;
    }
    else{
      break;
    }
  }
  fseek(arq, -1, SEEK_CUR);
}

void die(FILE* arq, char* error_msg){
  int err_pos = ftell(arq);
  int pos = 0;
  char c;
  char line_buffer[200];

  int line_number = 0, row_number = 0;
  fseek(arq, 0, SEEK_SET);

  while((c = fgetc(arq)) != EOF){

    if(c == '\n' || feof(arq)){
      if(pos >= err_pos){
        line_buffer[row_number] = '\0';
        // fprintf(stderr, "%s:%d:%d: %s\n", code_file_name, line_number+1, row_number+1, error_msg);
        fprintf(stderr, " %d |      %s\n", line_number+1, line_buffer);
        for(int i = 0; i < row_number-1; i++){
          line_buffer[i] = ' ';
        }
        line_buffer[row_number-1] = '^';
        line_buffer[row_number] = '\0';
        fprintf(stderr, "          %s\n", line_buffer);
        exit(EXIT_FAILURE);
      }
      line_number++, row_number = 0;
    }
    else{
      line_buffer[row_number++] = c;
    }
    pos++;
  }
}

void skip_spaces(FILE *arq){
  char c = fgetc(arq);
  while (c != EOF && isspace(c)) c = fgetc(arq);

  fseek(arq, -1, SEEK_CUR);
}

// Retorna true se encontra a string "expected_token", ignorando espaços
// antes da string. Caso não encontre a string, retorna false e volta
// o ponteiro *arq para a posição inicial
bool read_next_token(FILE *arq, char *expected_token, bool expect_comment){
  skip_spaces(arq);

  char c = fgetc(arq);
  if (expect_comment && c == '#')
    while (c != EOF && c != '\n') c = fgetc(arq);

  fseek(arq, -1, SEEK_CUR);

  skip_spaces(arq);

  char token[strlen(expected_token)+1];
  bool read = fread(token, strlen(expected_token), 1, arq);
  token[strlen(expected_token)] = '\0';
  bool found = strncmp(token, expected_token, strlen(expected_token)) == 0;
  if (!found) fseek(arq, -strlen(expected_token), SEEK_CUR);

  // puts(token);
  return found;
}

int read_number(FILE *arq, bool expect_spaces){
  char c = fgetc(arq);
  if (expect_spaces){
    while (c != EOF && isspace(c)){
      c = fgetc(arq);
    }
  }

  char buffer[10];
  int index = 0;
  while (isdigit(c)){
    buffer[index++] = c;
    c = fgetc(arq);
  }
  buffer[index] = '\0';

  if(!isdigit(c)) fseek(arq, -1, SEEK_CUR);

  return atoi(buffer);
}

bool read_uf(FILE *input, FILE *output, CPU_Configurations *cpu_configs){
  fprintf(output, "Lendo Uf\n");

  char *expected_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
  int num_ufs[num_tokens];
  memset(num_ufs, -1, sizeof(num_ufs));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_ufs[j] == -1
        && read_next_token(input, expected_tokens[j], false)
        && read_next_token(input, ":", false))
      {
        num_ufs[j] = read_number(input, true);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_ufs[i] == -1) return false;

  cpu_configs->size_add_ufs = num_ufs[0];
  cpu_configs->size_mul_ufs = num_ufs[1];
  cpu_configs->size_integer_ufs = num_ufs[2];
  yellow();
  fprintf(output, "ADD ufs: %d\n", cpu_configs->size_add_ufs);
  fprintf(output, "MUL ufs: %d\n", cpu_configs->size_mul_ufs);
  fprintf(output, "INTEGER ufs: %d\n", cpu_configs->size_integer_ufs);
  reset();
  // todo ->
  // cpu->add_ufs = malloc(sizeof(FunctionalUnit) * cpu->size_add_ufs);
  // for (int i=0; i<cpu->size_add_ufs; i++) cpu->add_ufs[i].current_cycle = 0;

  // cpu->mul_ufs = malloc(sizeof(FunctionalUnit) * cpu->size_mul_ufs);
  // for (int i=0; i<cpu->size_mul_ufs; i++) cpu->mul_ufs[i].current_cycle = 0;

  // cpu->integer_ufs = malloc(sizeof(FunctionalUnit) * cpu->size_integer_ufs);
  // for (int i=0; i<cpu->size_integer_ufs; i++) cpu->integer_ufs[i].current_cycle = 0;

  return true;
}

bool read_inst(FILE *input, FILE *output, CPU_Configurations *cpu_configs){
  fprintf(output, "Lendo inst\n");

  char *expected_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
  int num_cycles[num_tokens];
  memset(num_cycles, -1, sizeof(num_cycles));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_cycles[j] == -1
        && read_next_token(input, expected_tokens[j], false)
        && read_next_token(input, ":", false))
      {
        num_cycles[j] = read_number(input, true);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_cycles[i] == -1) return false;

  cpu_configs->cycles_to_complete_add = num_cycles[0];
  cpu_configs->cycles_to_complete_mul = num_cycles[1];
  cpu_configs->cycles_to_complete_integer = num_cycles[2];
  yellow();
  fprintf(output, "ADD cycles: %d\n", cpu_configs->cycles_to_complete_add);
  fprintf(output, "MUL cycles: %d\n", cpu_configs->cycles_to_complete_mul);
  fprintf(output, "INTEGER cycles: %d\n", cpu_configs->cycles_to_complete_integer);
  reset();
  return true;
}

// lê o arquivo de configurações, falta terminar
bool read_config(FILE *input, FILE *output, CPU_Configurations *cpu_configs)
{
  if (!read_next_token(input, "/*", true)) return false;

  char *next_tokens[] = { UF_SYMBOL, INST_SYMBOL };
  int num_tokens = sizeof(next_tokens) / sizeof(next_tokens[0]);
  bool completed[num_tokens];
  memset(completed, false, sizeof(completed));

  for(int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (read_next_token(input, next_tokens[j], false)){
        if (!read_next_token(input, ":", false)) return false;
        if (completed[j]) return false; // config duplicada

        switch (j){
          case 0:
            if (!read_uf(input, output, cpu_configs)) return false;
            break;
          case 1:
            if (!read_inst(input, output, cpu_configs)) return false;
            break;
        }
        completed[j] = true;
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (completed[i] == false) return false;
  
  return read_next_token(input, "*/", false);
}

bool parse_assembly(FILE *input, FILE *output, CPU_Configurations *cpu_configs, int *instruction_count, Byte **memory, int memory_size){
  fprintf(output, "Lendo configs\n");
  if (!read_config(input, output, cpu_configs)){
    fprintf(output, "Erro ao ler as configuracoes\n");
    return false;
  }
  fprintf(output, "Fim da leitura das configs\n");

  if (!read_data_section(input, memory, memory_size)){

    fprintf(output, ".data não encontrado\n");
    return false;
  }

  if (!read_next_token(input, ".text", true)){
    fprintf(output, ".text não encontrado\n");
    return false;
  }

  int instruction_code;
  int last_ftell = ftell(input);
  
  for(int i=0; (instruction_code = read_instruction(input, output)) != -1; i+=4){
    if (last_ftell == ftell(input)) break;
    last_ftell = ftell(input);


    (*memory)[i + PROGRAM_FIRST_ADDRESS + 0] = (instruction_code >> 24) & 0b11111111;
    (*memory)[i + PROGRAM_FIRST_ADDRESS + 1] = (instruction_code >> 16) & 0b11111111;
    (*memory)[i + PROGRAM_FIRST_ADDRESS + 2] = (instruction_code >> 8) & 0b11111111;
    (*memory)[i + PROGRAM_FIRST_ADDRESS + 3] = (instruction_code >> 0) & 0b11111111;
    (*instruction_count)++;
  }

  return true;
}

#endif