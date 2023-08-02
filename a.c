#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
// int add(int addres1, int addres2)
// {
//     return memory[addres1] + memory[addres2];
// }

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

void read_UF(FILE *arq)
{
    char buffer[1000];
    while (strcmp(fread(buffer, 1, 1, arq), "INST:") != 0)
    {
        if (strcmp(buffer, "add:") == 0)
            create_add_uf();
    }
}

void create_add_uf()
{

}

const int MAX_NUM_CONFIG_TOKENS = 1000;
const int MAX_CONFIG_TOKEN_LENGTH = 50;

void extract_config_text(FILE *arq, char* config_text){

  int char_idx=0;
  char ch;
  bool inside_comment = false;

  while((ch = fgetc(arq)) != EOF){

    if(inside_comment){

      if(ch == '*' && fgetc(arq) == '/'){
        inside_comment = false;
        break;
      }

      else
        config_text[char_idx++] = putchar(ch); 
    }
    else{
      if (ch == '/' && fgetc(arq) == '*')
        inside_comment = true;
    }
  }
  config_text[char_idx] = '\0';
  printf("%s\n", config_text);
  
}

void extract_tokens(char* text, const char* delimiters, char** tokens){

  int token_idx=0;
  char *token;

  token = strtok(text, delimiters);


  while(token != NULL){
    //printf("%s\n", token);
    tokens[token_idx++] = token;
    token = strtok(NULL, delimiters);
  }

}

void interpret_config(char** tokens){

}

void read_config(FILE *arq)
{
    char config_text[MAX_NUM_CONFIG_TOKENS*MAX_CONFIG_TOKEN_LENGTH];
    extract_config_text(arq, config_text);

    char* config_tokens[MAX_NUM_CONFIG_TOKENS];
    const char* delimiters = ": \t\n";
    extract_tokens(config_text, delimiters, config_tokens);

    interpret_config(config_tokens);
}

void read_instructions(FILE *arq)
{
  
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

int main(int argc, char *argv[])
{
    Functional_unit add;
    Functional_unit mul;
    Functional_unit integer;
 
    int memory_size=32;

    int memory[32];

    FILE *arq = fopen("input.sb", "rb");


    FILE* output = stdout;

    for(int i = 1; i < argc; i+=2){

      if(strcmp(argv[i], "-p") == 0){
        if((arq = fopen(argv[i+1], "r")) == NULL){
          printf("Falha na leitura do arquivo %s.\n", argv[i+1]);
        }
        read_config(arq);
        read_instructions(arq);
        printf("Arquivo %s lido com sucesso.\n", argv[i+1]);
      }
      
      else if(strcmp(argv[i], "-m") == 0){
        // TODO
        // validar que argv[i+1] é uma inteiro
        memory_size = atoi(argv[i+1]);
      }

      else if(strcmp(argv[i], "-o") == 0){
        output = fopen(argv[i+1], "w");
      }
    }

    fprintf(output, "se n tiver -o vai na saída padrão, senao vai no arquivo...\n");

    return 0;

}
