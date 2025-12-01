#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

extern float current_lux; 
extern const int lux_threshold;
extern int position;

void light_irq_handler(); 
void light_irq_init(); 
void light_init(); 
void read_lux(); 
void light_poll(); 



#endif