#ifndef TYPES
#define TYPES

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

typedef enum {
    FUNCTIONAL_UNIT,
    INSTRUCTION,
    REGISTER_INFO
} TableElementType;

typedef struct Instruction
{
    char* instruction_info;
    int issue_time;
    int read_operand_time;
    int exec_complete_time;
    int write_result_time;
} Instruction;

typedef struct RegisterInfo
{
    char* instruction_type;
} RegisterInfo;

typedef struct {
    TableElementType type;
    union {
        struct FunctionalUnit* functional_unit;
        struct Instruction* instruction;
        struct RegisterInfo* register_info;
    } data;
} Table_entry;

typedef struct {
    TableElementType type;
    int num_rows;
    Table_entry* table;
} Table;

typedef struct CPU
{
  FunctionalUnit *add_ufs;
  FunctionalUnit *mul_ufs;
  FunctionalUnit *integer_ufs;
  int size_add_ufs;
  int size_mul_ufs;
  int size_integer_ufs;
  int cycles_to_complete_add;
  int cycles_to_complete_mul;
  int cycles_to_complete_integer;
} CPU;

#endif