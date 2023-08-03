#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <cassert>
// int add(int addres1, int addres2)
// {
//     return memory[addres1] + memory[addres2];
// }

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

    return 0 | op | s | t | d | e;
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
      putchar(c);

      skip(arq);

      reading_uf_info = true;
      reading_inst_info = false;
    }
    if(strncmp(INST_SYMBOL, peek_buffer, strlen(INST_SYMBOL)) == 0){
      reading_uf_info = false;
      reading_inst_info = true;
    }

    else{
      die(arq, "No valid symbol found");
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

// verifica se a partir de SEEK_CUR, existe uma instrução
// por exemplo, "add rs,rs,rt" e retorna apenas seu opcode.
// se não existe nenhum, retorna -1
int read_instruction_name(FILE* arq){

  // pula espaço em branco no fim da execução
  skip(arq);
  return 0;
}

// retorna um inteiro que representa um operando do tipo type
// a partir de SEEK_CUR, se não exister nenhum, retorna -1;
int read_operand(FILE* arq, OPERAND_TYPE type){

  // pula espaço em branco no fim da execução
  skip(arq);
  return 0;
}



int read_instruction(int opcode, FILE* arq){

  int rd=0, rs=0, rt=0, imm=0, extra=0;

  switch(opcode){

    // tratar erros
    case 0:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 1:
      rt = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 2:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 3:
      rt = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 4:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 5:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 6:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 7:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 8:
      rd = read_operand(arq, REGISTER);
      rs = read_operand(arq, REGISTER);
      return read_instructionR(opcode, rs, rs, rt, 0);

    case 9:
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 10:
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 11:
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 12:
      rs = read_operand(arq, REGISTER);
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, IMM);
      return read_instructionI(opcode, rs, rt, imm);

    case 13:
      imm = read_operand(arq, IMM);
      return read_instructionJ(opcode, imm);

    case 14:
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, MEMORY);
      return read_instructionI(opcode, rs, rt, imm);

    case 15:
      rt = read_operand(arq, REGISTER);
      imm = read_operand(arq, MEMORY);
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
  read_config(arq);

  char c;

  while((c = fgetc(arq)) != EOF){

    int opcode=-1, section=-1;

    if((opcode = read_instruction_name(arq)) != -1){
      int instruction = read_instruction(opcode, arq);
      // 
    }

    else if((section = find_section(arq)) != -1){
      if(section == 0){
        read_data_section(arq);
      }
      else if(section == 1){
        continue;
      }
    }
  }



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
