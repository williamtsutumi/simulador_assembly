#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
// int add(int addres1, int addres2)
// {
//     return memory[addres1] + memory[addres2];
// }

#define int unsigned

int* memory;
int instruction_count = 0;

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

const char* MUL = "mul";
const char* ADD = "add";
const char* DIV = "div";
const char* SUB = "sub";
const char* JMP = "jmp";
const char* MOV = "sub";
const char* UF_SYMBOL = "UF";
const char* INST_SYMBOL = "INST";
const char* code_file_name;



void fpeek(FILE* arq, char* peekBuffer, int peekSize){

  char character;
  int i=0;
  fseek(arq, -1, SEEK_CUR);
  for(; i < peekSize; i++){

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
    else if(on_single_line_comment || c == ' ' 
        || c == '\t' || c == '\n'){
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

// lê o arquivo de configurações, falta terminar
void read_config(FILE *arq)
{
  
  char c;
  char peek_buffer[20];

  bool reading_uf_info = false;
  bool reading_inst_info = false;

  fpeek(arq, peek_buffer, 2);


  if(strncmp(peek_buffer, "/*", 2) != 0){
    die(arq, "Expected '/*'");
  }

  fseek(arq, 2, SEEK_CUR);

  while((c = fgetc(arq)) != EOF){
    fpeek(arq, peek_buffer, 10);

    if(strncmp(UF_SYMBOL, peek_buffer, strlen(UF_SYMBOL)) == 0){
      fseek(arq, strlen(UF_SYMBOL), SEEK_CUR);

      skip(arq);

      reading_uf_info = true;
      reading_inst_info = false;
    }
    if(strncmp(INST_SYMBOL, peek_buffer, strlen(INST_SYMBOL)) == 0){
      reading_uf_info = false;
      reading_inst_info = true;
    }

    if(strncmp("*/", peek_buffer, 2)){
      break;
    }
    else{
      continue;
      //die(arq, "No valid symbol found");
    }
  }

  skip(arq);
}

void decapitalize(char *str){
  int len = strlen(str);
  for(int i = 0; i < len; i++){
    if('A' <= str[i] && str[i] <= 'Z') str[i] -= 'A';
  }
}

int get_opcode(char* str){

  puts(str);
  if(strncmp("addi", str, 4) == 0)
    return 1;
  if(strncmp("subi", str, 4) == 0)
    return 3;
  if(strncmp("exit", str, 4) == 0)
    return 16;
  if(strncmp("add", str, 3) == 0)
    return 0;
  if(strncmp("sub", str, 3) == 0)
    return 2;
  if(strncmp("mul", str, 3) == 0)
    return 4;
  if(strncmp("div", str, 3) == 0)
    return 5;
  if(strncmp("and", str, 3) == 0)
    return 6;
  if(strncmp("or", str, 2) == 0)
    return 7;
  if(strncmp("not", str, 3) == 0)
    return 8;
  if(strncmp("blt", str, 3) == 0)
    return 9;
  if(strncmp("bgt", str, 3) == 0)
    return 10;
  if(strncmp("beq", str, 3) == 0)
    return 11;
  if(strncmp("bne", str, 3) == 0)
    return 12;
  if(strncmp("j", str, 1) == 0)
    return 13;
  if(strncmp("lw", str, 2) == 0)
    return 14;
  if(strncmp("sw", str, 2) == 0)
    return 15;
  else
    return -1;
  

}
bool validate_number(char *c){
  return true;
}

// verifica se a partir de SEEK_CUR, existe uma instrução
// por exemplo, "add rs,rs,rt" e retorna apenas seu opcode.
// se não existe nenhum, retorna -1
int read_instruction_name(FILE* arq){

  char buffer[10];
  char c;
  int cnt=0;

  while((c = fgetc(arq)) != EOF){
    if(c == ' '){
      break;
    }
    buffer[cnt++] = c;
  }
  buffer[cnt] = '\0';

  int opcode = get_opcode(buffer);
  printf("%d\n", opcode);

  skip(arq);
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

void parse_assembly(FILE *arq){
  skip(arq);
  //read_config(arq);

  char _;

  do{

    int opcode=-1, section=-1;

    if((opcode = read_instruction_name(arq)) != -1){
      printf("READ INSTRUCTION %d\n", opcode);
      int instruction = read_instruction(opcode, arq);
    }

    else if((section = find_section(arq)) != -1){
      if(section == 0){
        read_data_section(arq);
      }
      else if(section == 1){
        continue;
      }
    }
  }while((_ = fgetc(arq)) != EOF);



}


int main(int argc, char *argv[])
{
    code_file_name = argv[0];

    Functional_unit add;
    Functional_unit mul;
    Functional_unit integer;
 
    int memory_size=32;


    FILE* output_stream = stdout;

    char* file_name = "input.sb";

    for(int i = 1; i < argc; i+=2){

      if(strcmp(argv[i], "-p") == 0){
        file_name = argv[i+1];
      }
      
      else if(strcmp(argv[i], "-m") == 0){
        // TODO
        // validar que argv[i+1] é uma inteiro
        memory_size = atoi(argv[i+1]);
        memory = (int*)(malloc(sizeof(int)*memory_size));
      }

      else if(strcmp(argv[i], "-o") == 0){
        output_stream = fopen(argv[i+1], "w");
      }
    }

    FILE* arq;

    if((arq = fopen(file_name, "r")) == NULL){
      printf("Falha na leitura do arquivo %s.\n", file_name);
    }
    else{
      puts("lendo arquivo assembly...");
      parse_assembly(arq);
      printf("Arquivo %s lido com sucesso.\n", file_name);
    }


    fprintf(output_stream, "se n tiver -o vai na saída padrão, senao vai no arquivo...\n");

    return 0;

}
