
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/bus.h"

#define MAX_PULSE_QUEUE_SIZE 200

void add_pulse(Bus* bus, Pulse pulse){
  bus->pulse_queue[bus->pulse_count++] = pulse;
}

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
    bus->pulse_count = 0;
    bus->current_pulse = 0;
}

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


