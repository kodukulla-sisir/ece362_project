#ifndef PWM_H
#define PWM_H

#include <stdbool.h>

void my_gpio_init(bool dir); 
void my_pwm_init(bool dir); 
void motor_two();


#endif