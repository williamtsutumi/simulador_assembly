#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

void read_config(FILE *arq)
{
    char buffer[1000];

    fread(buffer, 2, 1, arq);
    if (strcmp(buffer, "/*") != 0)
        return;

    fseek(arq, 2, SEEK_CUR);
    fread(buffer, 3, 1, arq);
    if (strcmp(buffer, "UF:") != 0)
        return;

    printf("A\n");
    read_UF(arq);
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
 
    int memory[32];
    FILE *arq = fopen("input.sb", "rb");

    read_config(arq);

    return 0;
}