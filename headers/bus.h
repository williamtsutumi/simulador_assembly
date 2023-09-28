#ifndef BUS
#define BUS

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

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
void add_pulse(Bus* bus, Pulse pulse){
  bus->pulse_queue[bus->pulse_count++] = pulse;
}

// envia todos os pulsos pendententes
void dispatch_pulses(Bus* bus){
    printf("dispachando %d pulsos\n", bus->pulse_count);

    while(bus->current_pulse < bus->pulse_count){
        int curr = bus->current_pulse;
        
        if(bus->pulse_queue[curr].has_data){
            printf("pulso de dados\n");
            printf("dado enviado: %d\n", bus->pulse_queue[curr].data);
            memcpy(bus->pulse_queue[curr].to.endpoint, &(bus->pulse_queue[curr].data), bus->pulse_queue[curr].size);

        }
        else{
            printf("pulso de pulso\n");
            printf("dado enviado: %d\n", bus->pulse_queue[curr].from.endpoint);
            memcpy(bus->pulse_queue[curr].to.endpoint, bus->pulse_queue[curr].from.endpoint, bus->pulse_queue[curr].size);
        }

        bus->current_pulse++;
    }
    bus->pulse_count = bus->current_pulse = 0;
}

// cria um pulso, o valor em *from será copiado para o local to de memória
Pulse new_pulse(void* from, void* to, size_t size){

    Endpoint from_ed, to_ed;
    from_ed.endpoint = from;
    to_ed.endpoint = to;

    Pulse pulse;
    pulse.from = from_ed;
    pulse.to = to_ed;
    pulse.size = size;

    return pulse;
}

// o valor data será copiado para o local to
Pulse new_data_pulse(unsigned data, void* to, size_t size){
    Endpoint to_ed;
    to_ed.endpoint = to;

    Pulse pulse;
    pulse.has_data = true;
    pulse.data = data;
    pulse.size = size;
    pulse.to = to_ed;

    return pulse;
}




#endif