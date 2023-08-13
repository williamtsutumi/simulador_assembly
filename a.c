#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define ADD "add"
#define ADD_OPCODE 0
#define ADDI "addi"
#define ADDI_OPCODE 1
#define SUB "sub"
#define SUB_OPCODE 2
#define SUBI "subi"
#define SUBI_OPCODE 3
#define MUL "mul"
#define MUL_OPCODE 4
#define DIV "div"
#define DIV_OPCODE 5
#define AND "and"
#define AND_OPCODE 6
#define OR "or"
#define OR_OPCODE 7
#define NOT "not"
#define NOT_OPCODE 8
#define BLT "blt"
#define BLT_OPCODE 9
#define BGT "bgt"
#define BGT_OPCODE 10
#define BEQ "beq"
#define BEQ_OPCODE 11
#define BNE "bne"
#define BNE_OPCODE 12
#define J "j"
#define J_OPCODE 13
#define LW "lw"
#define LW_OPCODE 14
#define SW "sw"
#define SW_OPCODE 15
#define EXIT "exit"
#define EXIT_OPCODE 16

// Símbolos para leitura das configurações
#define UF_SYMBOL "UF"
#define INST_SYMBOL "INST"
#define ADD_SYMBOL "add"
#define MUL_SYMBOL "mul"
#define INTEGER_SYMBOL "integer"
#define CONFIG_SYMBOLS { ADD_SYMBOL, MUL_SYMBOL, INTEGER_SYMBOL }

typedef enum UF_type
{
    add,
    mul,
    integer,
} UF_type;

typedef struct Functional_unit
{
    UF_type type;
    int cycles;

} Functional_unit;

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

int* memory;
int instruction_count = 0;
const char* code_file_name;

int add_ufs, mul_ufs, integer_ufs;
int add_cycles, mul_cycles, integer_cycles;
Functional_unit *functional_units;

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
        fprintf(stderr, "%s:%d:%d: %s\n", code_file_name, line_number+1, row_number+1, error_msg);
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

  char token[strlen(expected_token)];
  fread(token, strlen(expected_token), 1, arq);
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
  return atoi(buffer);
}

bool read_uf(FILE *input, FILE *output){
  fprintf(output, "Lendo Uf\n");

  char *expeted_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expeted_tokens) / sizeof(expeted_tokens[0]);
  int num_ufs[num_tokens];
  memset(num_ufs, -1, sizeof(num_ufs));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_ufs[j] == -1
        && read_next_token(input, expeted_tokens[j])
        && read_next_token(input, ":"))
      {
        num_ufs[j] = read_number(input);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_ufs[i] == -1) return false;

  add_ufs = num_ufs[0];
  mul_ufs = num_ufs[1];
  integer_ufs = num_ufs[2];
  Functional_unit ufs[add_ufs + mul_ufs + integer_ufs];
  

  for (int i=0; i<add_ufs; i++){
    ufs[i].type = add;
    ufs[i].cycles = -1;
  }
  for (int i=add_ufs; i<add_ufs+mul_ufs; i++){
    ufs[i].type = mul;
    ufs[i].cycles = -1;
  }
  for (int i=add_ufs+mul_ufs; i<add_ufs+mul_ufs+integer_ufs; i++){
    ufs[i].type = integer;
    ufs[i].cycles = -1;
  }
  for(int i=0; i<add_ufs+mul_ufs+integer_ufs; i++){
    printf("%d\n", ufs[i].type);
  }
  printf("-------------\n");
  functional_units = ufs;
  for(int i=0; i<add_ufs+mul_ufs+integer_ufs; i++){
    printf("%d\n", functional_units[i].type);
  }
  yellow();
  fprintf(output, "ADD ufs: %d\n", add_ufs);
  fprintf(output, "MUL ufs: %d\n", mul_ufs);
  fprintf(output, "INTEGER ufs: %d\n", integer_ufs);
  reset();
  return true;
}

bool read_inst(FILE *input, FILE *output){
  fprintf(output, "Lendo inst\n");

  char *expeted_tokens[] = CONFIG_SYMBOLS;
  int num_tokens = sizeof(expeted_tokens) / sizeof(expeted_tokens[0]);
  int num_cycles[num_tokens];
  memset(num_cycles, -1, sizeof(num_cycles));

  for (int i=0; i<num_tokens; i++){
    for (int j=0; j<num_tokens; j++){
      if (
        num_cycles[j] == -1
        && read_next_token(input, expeted_tokens[j])
        && read_next_token(input, ":"))
      {
        num_cycles[j] = read_number(input);
      }
    }
  }
  for (int i=0; i<num_tokens; i++)
    if (num_cycles[i] == -1) return false;

  add_cycles = num_cycles[0];
  mul_cycles = num_cycles[1];
  integer_cycles = num_cycles[2];
  yellow();
  fprintf(output, "ADD cycles: %d\n", add_cycles);
  fprintf(output, "MUL cycles: %d\n", mul_cycles);
  fprintf(output, "INTEGER cycles: %d\n", integer_cycles);
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

bool validate_number(char *c){
  return true;
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

// retorna um inteiro que representa um operando do tipo type
// a partir de SEEK_CUR, se não exister nenhum, retorna -1;
int read_operand(FILE* arq, OPERAND_TYPE type, bool expect_comma){
  
  puts("here");


  if(type == REGISTER){
    int register_id = read_register_id(arq);
    skip(arq);
    if(expect_comma){
      char comma = fgetc(arq);
      if(comma != ','){
        die(arq, "Expected ','");
      }
    }
    skip(arq);

    return register_id;
  }
  else if(type == IMM){
    return -1;
  }
  else if(type == MEMORY){
    return -1;
  }

  // pula espaço em branco no fim da execução
  skip(arq);

  return 0;
}

int dec_to_bin(int num){

  for(int i = 0; i < 32; i++){
    printf("%d", (num&(1 << (31-i)))!=0);
  }
  putchar('\n');

}

int read_instruction(int opcode, FILE* arq){

  int rd=0, rs=0, rt=0, imm=0, extra=0;

  switch(opcode){

    // tratar erros
    case 0:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);

      printf("%d %d %d\n", rd, rs, rt);
      printf("%d\n", read_instructionR(opcode, rd, rs, rt, 0));
      die(arq, "Morri");
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 1:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 2:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 3:
      rt = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 4:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);

      printf("%d %d %d\n", rd, rs, rt);
      printf("%d\n", read_instructionR(opcode, rd, rs, rt, 0));

      dec_to_bin(read_instructionR(opcode, rd, rs, rt, 0));
      die(arq, "morri");

      return read_instructionR(opcode, rs, rs, rt, 0);

    case 5:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 6:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 7:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 8:
      rd = read_operand(arq, REGISTER, true);
      rs = read_operand(arq, REGISTER, false);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 9:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 10:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 11:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 12:
      rs = read_operand(arq, REGISTER, true);
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, IMM, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 13:
      imm = read_operand(arq, IMM, false);
      return read_instructionJ(opcode, imm);

    case 14:
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, MEMORY, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 15:
      rt = read_operand(arq, REGISTER, true);
      imm = read_operand(arq, MEMORY, false);
      return read_instructionI(opcode, rs, rt, imm);

    case 16:
      return read_instructionJ(opcode, 1);
  }

  assert(false);
  return -1;

}

// verifica se na posição SEEK_CUR do arq, existe uma string do tipo 
// .secao, onde secao é o nome da seção, se o nome é .data, retorna 0,
// se for .text, retorna 1, se não tiver seção, retorna -1;
// 
int find_section(FILE *arq){

  skip(arq);
  return -1;
}

// Lê as informações da seção .data, que está no SEEK_CUR
void read_data_section(FILE *arq){

  skip(arq);
}

void parse_assembly(FILE *input, FILE *output){
  // skip(arq);
  // fprintf(output, "Lendo configs\n");
  // if (!read_config(input, output)){
  //   fprintf(output, "Erro ao ler as configuracoes\n");
  //   return;
  // }
  // fprintf(output, "Fim da leitura das configs\n");


  char _;

  do{
    // printf("A\n");
    int opcode=-1, section=-1;
    printf("ftell: %li\n", ftell(input));
    if((opcode = read_instruction_name(input, output)) != -1){
      fprintf(output, "READ INSTRUCTION %d\n", opcode);
      int instruction = read_instruction(opcode, input);
    }
    else if((section = find_section(input)) != -1){
      if(section == 0){
        read_data_section(input);
      }
      else if(section == 1){
        continue;
      }
    }
  }while((_ = fgetc(input)) != EOF);

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

int main(int argc, char *argv[])
{
  code_file_name = argv[0];
  // Functional_unit add;
  // Functional_unit mul;
  // Functional_unit integer;

  int memory_size = 32;
  FILE* output_stream = stdout;
  char* input_file_name = "input.txt";
  FILE* input_file;
  
  if (read_args(argc, argv, &memory_size, &input_file_name, &input_file, &output_stream)){
    fprintf(output_stream, "Lendo arquivo %s ...\n", input_file_name);
    parse_assembly(input_file, output_stream);
  }
  else{
    fprintf(output_stream, "Falha na leitura do arquivo %s.\n", input_file_name);
  }

  fprintf(output_stream, "se n tiver -o vai na saída padrão, senao vai no arquivo...\n");

  printf("printando functional_units na main\n");
  for (int i=0; i<add_ufs+mul_ufs+integer_ufs; i++){
    printf("%d\n", functional_units[i].type);
  }

  return 0;

}
