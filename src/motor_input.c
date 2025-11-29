#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

const int thresh = 0.11; // Example threshold value
//counter clockwise is dir = 0;
// clockwise is dir  = 1;
//ccw increases light before 180 deg;
// cw decreases light before 180 deg;
int motor_input_init(int val)
{
    int dir;
    if (val < thresh) {
        dir = 0; // counter clockwise
    } 
    else {
        dir = 1; // clockwise
    }
    return dir;
}   

int motor_input_run(int dir)
{
    my_pwm_init(dir);
    
}

