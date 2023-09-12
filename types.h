#ifndef TYPES
#define TYPES

#define MAX_NUM_INSTRUCTIONS 50
#define MAX_NUM_ROWS_TABLE 100

/* TYPES para leitura do input */

typedef enum OPERAND_TYPE
{
    REGISTER,
    IMM,
    MEMORY,
} OPERAND_TYPE;

/* TYPES da representação das unidades funcionais */

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

// if (bus. sinal de continuar) current_cycle++;

/* TYPES do scoreboarding */

typedef enum InstructionState {
    FETCH,
    ISSUE,
    READ_OPERANDS,
    EXECUTE,
    WRITE_RESULT
} InstructionState;

    typedef struct FunctionalUnitState
    {
        UF_TYPE type;
        char* op, fi, fj, fk, qj, qk, rj, rk;
        bool busy;
    } FunctionalUnitState;

        typedef struct ScoreBoard
        {
            FunctionalUnitState *ufs_states;
            InstructionState instructions_states[MAX_NUM_INSTRUCTIONS];
            // Representa qual unidade funcional escreverá em qual registrador.
            // Qualquer instrução deve dar stall caso o registrador de destino
            // já esteja sendo sofrendo escrita por outra instrução.
            FunctionalUnit *result_register_state[/*numero de registradores*/];
        } ScoreBoard;

/* TYPES do barramento */

// Indica o que deve um componente deve fazer: continuar sua execução ou permanecer parado
typedef enum ControlSignal {
    CONTINUE,
    STALL
} ControlSignal;

// Indica se a informação no barramento deve ser lido ou não
typedef enum SignalFlag {
    IGNORE,
    WRITE_TO_DESTINATION
} SignalFlag;

    // Indica um dado se movendo de um componente a outro, representa uma parte do barramento
    typedef struct DataSignal {
        int data;
        SignalFlag flag;
    } DataSignal;

        typedef struct Bus {
            DataSignal regs[32]; // Informação sendo enviada aos registradores
            DataSignal uf; // Informação sendo enviada às unidades funcionais
            ControlSignal instruction_register;
            ControlSignal uf_state1;
        } Bus;


/* NAO SEI OQ EH ISSO */

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

/* GERAL */

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