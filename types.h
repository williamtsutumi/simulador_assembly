#ifndef TYPES
#define TYPES

#define MAX_NUM_INSTRUCTIONS 50
#define MAX_NUM_ROWS_TABLE 100

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

typedef enum {
    INTEGER_UF,
    MUL_UF,
    ADD_UF
} UF_TYPE;

typedef struct FunctionalUnit
{
    UF_TYPE type;
    int current_cycle;
    char* name;
} FunctionalUnit;

typedef struct DataBus
{
    int nao_sei_oq_botar;
} DataBus;

// Existem três partes no scoreboard:
// 1. Estado da instrução. Indica em qual das quatro etapas a instrução está.
// 2. Estado da unidade funcional. Indica o estado da unidade funcional. Existem nove 
// campos para cada unidade funcional:
// j Busy. Indica se a unidade está ocupada ou não.
// j Op. Operação a realizar na unidade (p. ex., adição ou subtração).
// j Fi. Registrador destino.
// j Fj, Fk. Números de registrador-fonte.
// j Qj, Qk. Unidades funcionais produzindo registradores-fonte Fj, Fk.
// j Rj, Rk. Flags indicando quando Fj, Fk estão prontos e ainda não lidos. Definido 
// como Não após os operandos serem lidos.
// 3. Estado de resultado de registrador. Indica qual unidade funcional escreverá em 
// cada registrador, se uma instrução ativa tiver o registrador como seu destino. Esse 
// campo é definido como um espaço em branco sempre que não houver instruções 
// pendentes que escreverão nesse registrador.
typedef struct InstructonState
{

} InstructonState;

typedef struct FunctionalUnitState
{
    UF_TYPE type;
    char* op, fi, fj, fk, qj, qk, rj, rk;
    bool busy;
} FunctionalUnitState;

typedef struct ResultRegisterState
{

} ResultRegisterState;

typedef enum InstructionState {
    ISSUE,
    READ_OPERANDS,
    EXECUTE,
    WRITE_RESULT
} InstructionState;

typedef struct ScoreBoard
{
    FunctionalUnitState *ufs_states;
    InstructionState instructions_states[MAX_NUM_INSTRUCTIONS];
    FunctionalUnit *result_register_state[/*numero de registradores*/];
} ScoreBoard;

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

typedef struct CPU_Configurations
{
  int size_add_ufs;
  int size_mul_ufs;
  int size_integer_ufs;
  int cycles_to_complete_add;
  int cycles_to_complete_mul;
  int cycles_to_complete_integer;
} CPU_Configurations;

#endif