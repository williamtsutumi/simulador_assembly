#ifndef BUS_H
#define BUS_H

#define MAX_PULSE_QUEUE_SIZE 200


// referência a uma variável que representa a extremidade de um componente,
// ex: PC, o valor do operador 1 de uma UF, um local na memória, etc.
typedef struct Endpoint{
    void* endpoint;
} Endpoint;

// um pulso elétrico
typedef struct Pulse{
    size_t size;
    Endpoint from, to;

    // é possível que não exista um ponteiro para from, então o dado
    // é copiado direto para to, ao invés de ser copiado de from pra to.
    bool has_data;
    int data;
} Pulse;

// o barramento é uma fila de pulsos, ele não conhece
// nenhuma outra parte do código.
typedef struct Bus{
    Pulse pulse_queue[MAX_PULSE_QUEUE_SIZE];
    int pulse_count, current_pulse;
} Bus;

// coloca um pulso no barramento
void add_pulse(Bus* bus, Pulse pulse);

// envia todos os pulsos pendententes
void dispatch_pulses(Bus* bus);

// cria um pulso, o valor em *from será copiado para o local to de memória
Pulse new_pulse(void* from, void* to, size_t size);

// o valor data será copiado para o local to
Pulse new_data_pulse(unsigned data, void* to, size_t size);




#endif