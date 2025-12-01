#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H


void light_irq_handler(); 
void light_irq_init(); 
void light_init(); 
void read_lux(); 
void light_poll(); 

#endif