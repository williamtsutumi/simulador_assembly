#ifndef ASSEMBLY_PARSE
#define ASSEMBLY_PARSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define ADD "add "
#define ADD_OPCODE 0
#define ADDI "addi "
#define ADDI_OPCODE 1
#define SUB "sub "
#define SUB_OPCODE 2
#define SUBI "subi "
#define SUBI_OPCODE 3
#define MUL "mul "
#define MUL_OPCODE 4
#define DIV "div "
#define DIV_OPCODE 5
#define AND "and "
#define AND_OPCODE 6
#define OR "or "
#define OR_OPCODE 7
#define NOT "not "
#define NOT_OPCODE 8
#define BLT "blt "
#define BLT_OPCODE 9
#define BGT "bgt "
#define BGT_OPCODE 10
#define BEQ "beq "
#define BEQ_OPCODE 11
#define BNE "bne "
#define BNE_OPCODE 12
#define J "j "
#define J_OPCODE 13
#define LW "lw "
#define LW_OPCODE 14
#define SW "sw "
#define SW_OPCODE 15
#define EXIT "exit "
#define EXIT_OPCODE 16
#define INSTRUCTION_NAMES { ADD, ADDI, SUB, SUBI, MUL, DIV, AND, OR, NOT, BLT, BGT, BEQ, BNE, J, LW, SW, EXIT }


// Símbolos para leitura das configurações
#define UF_SYMBOL "UF"
#define INST_SYMBOL "INST"
#define ADD_SYMBOL "add"
#define MUL_SYMBOL "mul"
#define INTEGER_SYMBOL "integer"
#define CONFIG_SYMBOLS { ADD_SYMBOL, MUL_SYMBOL, INTEGER_SYMBOL }

#define NUM_MAX_INSTRUCTIONS 50

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

typedef struct FunctionalUnit
{
    int current_cycle;
    char* name;
    bool busy;
    char* op, fi, fj, fk, qj, qk, rj, rk;
} FunctionalUnit;


int read_operand(FILE* arq, OPERAND_TYPE type, bool expect_comma);
int read_number(FILE *arq);
bool validate_number(char *c);
bool read_next_token(FILE *arq, char *expected_token);
void skip(FILE *arq);
void die(FILE* arq, char* error_msg);

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


void red () {
  printf("\033[1;31m");
}
void yellow() {
  printf("\033[1;33m");
}
void reset () {
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
  if(strncmp(ADDI, str, strlen(ADDI)) == 0)
    return ADDI_OPCODE;
  if(strncmp(SUBI, str, strlen(SUBI)) == 0)
    return SUBI_OPCODE;
  if(strncmp(EXIT, str, strlen(EXIT)) == 0)
    return EXIT_OPCODE;
  if(strncmp(ADD, str, strlen(ADD)) == 0)
    return ADD_OPCODE;
  if(strncmp(SUB, str, strlen(SUB)) == 0)
    return SUB_OPCODE;
  if(strncmp(MUL, str, strlen(MUL)) == 0)
    return MUL_OPCODE;
  if(strncmp(DIV, str, strlen(DIV)) == 0)
    return DIV_OPCODE;
  if(strncmp(AND, str, strlen(AND)) == 0)
    return AND_OPCODE;
  if(strncmp(NOT, str, strlen(NOT)) == 0)
    return NOT_OPCODE;
  if(strncmp(BLT, str, strlen(BLT)) == 0)
    return BLT_OPCODE;
  if(strncmp(BGT, str, strlen(BGT)) == 0)
    return BGT_OPCODE;
  if(strncmp(BEQ, str, strlen(BEQ)) == 0)
    return BEQ_OPCODE;
  if(strncmp(BNE, str, strlen(BNE)) == 0)
    return BNE_OPCODE;
  if(strncmp(OR, str, strlen(OR)) == 0)
    return OR_OPCODE;
  if(strncmp(LW, str, strlen(LW)) == 0)
    return LW_OPCODE;
  if(strncmp(SW, str, strlen(SW)) == 0)
    return SW_OPCODE;
  if(strncmp(J, str, strlen(J)) == 0)
    return J_OPCODE;
  else
    return -1;
  

}

int read_instruction_given_opcode(int opcode, FILE* arq){

  int rd=0, rs=0, rt=0, imm=0, extra=0;

  switch(opcode){

    // tratar erros
    case ADD_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case ADDI_OPCODE:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case SUB_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case SUBI_OPCODE:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case MUL_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);

      return read_instructionR(opcode, rs, rs, rt, 0);

    case DIV_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case AND_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case OR_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case NOT_OPCODE:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

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
      imm = read_operand(arq, MEMORY, false);
      return read_instructionI(opcode, rs, rt, imm);

    case SW_OPCODE:
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, MEMORY, false);
      return read_instructionI(opcode, rs, rt, imm);

    case EXIT_OPCODE:
      return read_instructionJ(opcode, 1);
  }

  assert(false);
  return -1;

}
// Lê as informações da seção .data, que está no SEEK_CUR
void read_data_section(FILE *arq){

  skip(arq);
}



int read_instruction(FILE *arq, FILE *output){

  // fprintf(output, "Lendo Instruções\n");


  char *expected_tokens[] = INSTRUCTION_NAMES;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);

  for (int opcode=0; opcode < num_tokens; opcode++){

    // printf("BUSCANDO %s\n", expected_tokens[opcode]);
    if(read_next_token(arq, expected_tokens[opcode])){
      printf("LEU %s\n", expected_tokens[opcode]);
      return read_instruction_given_opcode(opcode, arq);
    }
  }

  return -1;

}

// retorna um inteiro que representa um operando do tipo type
// a partir de SEEK_CUR, se não exister nenhum, retorna -1;
int read_operand(FILE* arq, OPERAND_TYPE type, bool expect_comma){
  

  if(type == REGISTER){
    if(!read_next_token(arq, "r")) return -1;

    int register_id = read_number(arq);
    if(expect_comma && !read_next_token(arq, ",")){
      die(arq, "Expected ','");
    }

    return register_id;
  }
  else if(type == IMM){
    return read_number(arq);
  }
  else if(type == MEMORY){
    int desvio = read_number(arq);
    if(!read_next_token(arq, "("))
      die(arq, "Expected '('");
    if(!read_next_token(arq, "r"))
      die(arq, "Expected 'r'");
    int register_id = read_number(arq);

    if(!read_next_token(arq, ")"))
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

// Retorna true se encontra a string "expexted_token", ignorando espaços
// antes da string. Caso não encontre a string, retorna false e volta
// o ponteiro *arq para a posição inicial
bool read_next_token(FILE *arq, char *expected_token){
  char c = fgetc(arq);
  while (c != EOF && isspace(c)){
    c = fgetc(arq);
  }
  fseek(arq, -1, SEEK_CUR);

  char token[strlen(expected_token)+1];
  fread(token, strlen(expected_token), 1, arq);
  token[strlen(expected_token)] = '\0';
  bool found = strncmp(token, expected_token, strlen(expected_token)) == 0;
  if (!found) fseek(arq, -strlen(expected_token), SEEK_CUR);

  return found;
}

// falta verificar se tem não digitos entre os dígitos também
// precisar retornar true/false se conseguiu ler número
int read_number(FILE *arq){
  char c = fgetc(arq);
  while (c != EOF && isspace(c)){
    c = fgetc(arq);
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

bool read_uf(FILE *input, FILE *output){
  fprintf(output, "Lendo Uf\n");

  char *expected_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
  int num_ufs[num_tokens];
  memset(num_ufs, -1, sizeof(num_ufs));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_ufs[j] == -1
        && read_next_token(input, expected_tokens[j])
        && read_next_token(input, ":"))
      {
        num_ufs[j] = read_number(input);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_ufs[i] == -1) return false;

  int add_ufs = num_ufs[0];
  int mul_ufs = num_ufs[1];
  int integer_ufs = num_ufs[2];
  yellow();
  fprintf(output, "ADD ufs: %d\n", add_ufs);
  fprintf(output, "MUL ufs: %d\n", mul_ufs);
  fprintf(output, "INTEGER ufs: %d\n", integer_ufs);
  reset();
//   cpu.add_ufs = malloc(sizeof(FunctionalUnit) * add_ufs);
//   printf("çalksdjfakj\n");
//   printf("%li\n", sizeof(cpu.add_ufs) / sizeof(cpu.add_ufs[0]));
  for (int i=0; i<add_ufs; i++){
    // cpu.add_ufs[i].current_cycle = 0;
  }
//   cpu.mul_ufs = malloc(sizeof(FunctionalUnit) * mul_ufs);
//   printf("%li\n", sizeof(cpu.mul_ufs) / sizeof(cpu.mul_ufs[0]));
  for (int i=0; i<mul_ufs; i++){
    // cpu.mul_ufs[i].current_cycle = 0;
  }
//   cpu.integer_ufs = malloc(sizeof(FunctionalUnit) * integer_ufs);
//   printf("%li\n", sizeof(cpu.integer_ufs) / sizeof(cpu.integer_ufs[0]));
  for (int i=0; i<integer_ufs; i++){
    // cpu.integer_ufs[i].current_cycle = 0;
  }

  
  return true;
}

bool read_inst(FILE *input, FILE *output){
  fprintf(output, "Lendo inst\n");

  char *expected_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expected_tokens) / sizeof(expected_tokens[0]);
  int num_cycles[num_tokens];
  memset(num_cycles, -1, sizeof(num_cycles));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_cycles[j] == -1
        && read_next_token(input, expected_tokens[j])
        && read_next_token(input, ":"))
      {
        num_cycles[j] = read_number(input);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_cycles[i] == -1) return false;

//   add_cycles = num_cycles[0];
//   mul_cycles = num_cycles[1];
//   integer_cycles = num_cycles[2];
  yellow();
//   fprintf(output, "ADD cycles: %d\n", add_cycles);
//   fprintf(output, "MUL cycles: %d\n", mul_cycles);
//   fprintf(output, "INTEGER cycles: %d\n", integer_cycles);
  reset();
  return true;
}

// lê o arquivo de configurações, falta terminar
bool read_config(FILE *input, FILE *output)
{
  if (!read_next_token(input, "/*")) return false;

  char *next_tokens[] = { UF_SYMBOL, INST_SYMBOL };
  int num_tokens = sizeof(next_tokens) / sizeof(next_tokens[0]);
  bool completed[num_tokens];
  memset(completed, false, sizeof(completed));

  for(int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (read_next_token(input, next_tokens[j])){
        if (!read_next_token(input, ":")) return false;
        if (completed[j]) return false; // config duplicada

        switch (j){
          case 0:
            if (!read_uf(input, output)) return false;
            break;
          case 1:
            if (!read_inst(input, output)) return false;
            break;
        }
        completed[j] = true;
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (completed[i] == false) return false;
  
  return read_next_token(input, "*/");
}

bool parse_assembly(FILE *input, FILE *output){
  // skip(arq);
  fprintf(output, "Lendo configs\n");
  if (!read_config(input, output)){
    fprintf(output, "Erro ao ler as configuracoes\n");
    return false;
  }
  fprintf(output, "Fim da leitura das configs\n");

  if (!read_next_token(input, ".data")){
    fprintf(output, ".data não encontrado\n");
    return false;
  }

  if (!read_next_token(input, ".text")){
    fprintf(output, ".text não encontrado\n");
    return false;
  }

//   instructions = (int *)malloc(NUM_MAX_INSTRUCTIONS * sizeof(int));
//   memset(instructions, -1, sizeof(instructions));
  int instruction_code;
  for(int i=0; (instruction_code = read_instruction(input, output)) != -1; i++){
    // instructions[i] = instruction_code;
  }
  return true;
}

#endif