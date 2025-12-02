#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "pico/rand.h"
#include "lightsensor.h"
#include "pwm.h"

extern const int ADDR;// = 0x51;
extern const int LOWER_LUX;// = 50;
extern const int HIGH_LUX; // = 400;
extern const int LOW_THRESH; // = LOWER_LUX / 0.012;
extern const int HIGH_THRESH; // = HIGH_LUX / 0.012;
extern const int INT_PIN; // = 16;
extern const int PS_HIGH_THRESH;

const int poll_rate_us = 1000000; // 1 second
//float current_lux = 0.0f;
float current_lux = 0.0f; 
const int lux_threshold = 300; 
int PS_threshold = 0;

// void light_irq_handler()
// {
//     gpio_acknowledge_irq(INT_PIN, GPIO_IRQ_EDGE_FALL);
//     uint8_t rD = 0xD;
//     uint8_t regD[2];

//     i2c_write_blocking(i2c1, ADDR, &rD, 1, true);
//     i2c_read_blocking(i2c1, ADDR, regD, 2, false);

//     if (regD[1] & (1 << 5))
//     {
//         //my_pwm_init(false, true);
//         printf("ALS low threshold interrupt\n");
//     }

//     if (regD[1] & (1 << 4))
//     {
//         printf("ALS high threshold interrupt\n");
//     }

//     return;
// }

// void light_irq_init()
// {
//     gpio_init(INT_PIN);
//     gpio_set_dir(INT_PIN, false);
//     gpio_set_function(INT_PIN, GPIO_FUNC_SIO);
//     gpio_set_irq_enabled_with_callback(INT_PIN, GPIO_IRQ_EDGE_FALL, true, light_irq_handler);

//     return;
// }

void sensor_irq_handler()
{
    gpio_acknowledge_irq(INT_PIN, GPIO_IRQ_EDGE_FALL);
    uint8_t rD = 0xD;
    uint8_t regD[2];

    i2c_write_blocking(i2c1, ADDR, &rD, 1, true);
    i2c_read_blocking(i2c1, ADDR, regD, 2, false);
    
    int test = regD[1] & (1 << 1);
    printf("Val: %d\n", test);
    if (regD[1] & (1 << 1))
    {
        //my_pwm_init(false, true);
        PS_threshold = 1;
        printf("PS_Threshold: %d\n",PS_threshold);
    }
    else
    {
        PS_threshold = 0;
    }

    return;
}

void sensor_irq_init()
{
    gpio_init(INT_PIN);
    gpio_set_dir(INT_PIN, false);
    gpio_set_function(INT_PIN, GPIO_FUNC_SIO);
    gpio_set_irq_enabled_with_callback(INT_PIN, GPIO_IRQ_EDGE_FALL, true, sensor_irq_handler);
}

void light_init ()
{
    // Init
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    i2c_init(i2c1, 100000);

    // Control Register
    uint8_t reg0[3];
    reg0[0] = 0x00;
    reg0[1] = 0b01000110;
    reg0[2] = 0x00;
    i2c_write_blocking(i2c1, ADDR, reg0, 3, false);

    // Lower Threshold Register
    uint8_t reg1[3];
    reg1[0] = 0x02;
    reg1[1] = LOW_THRESH & 0xFF;
    reg1[2] = (LOW_THRESH >> 8) & 0xFF; 
    i2c_write_blocking(i2c1, ADDR, reg1, 3, false);

    // Higher Threshold Register
    uint8_t reg2[3];
    reg2[0] = 0x01;
    reg2[1] = HIGH_THRESH & 0xFF;
    reg2[2] = (HIGH_THRESH >> 8) & 0xFF; 
    i2c_write_blocking(i2c1, ADDR, reg2, 3, false);

    // PS Config1 and Config2 Register
    uint8_t reg3[3];
    reg3[0] = 0x03;
    reg3[1] = 0b00010110;
    reg3[2] = 0b00001001;
    i2c_write_blocking(i2c1, ADDR, reg3, 3, false);

    // PS Config3 and MS Register
    uint8_t reg4[3];
    reg4[0] = 0x04;
    reg4[1] = 0b01010011;
    reg4[2] = 0b00000100;
    i2c_write_blocking(i2c1, ADDR, reg4, 3, false);
    
    // Configure the PS Threshold
    uint8_t reg7[3];
    reg7[0] = 0x06;
    reg7[1] = PS_HIGH_THRESH & 0xFF;
    reg7[2] = (PS_HIGH_THRESH >> 8) & 0xFF; 
    i2c_write_blocking(i2c1, ADDR, reg7, 3, false);

    //light_irq_init();
    return;
}


void read_lux()
{
    hw_clear_bits(&timer_hw->intr, 1u << 0); 
    uint8_t reg = 0x09; // ALS_DATA register
    uint8_t data[2] = {0, 0};

    int w = i2c_write_blocking(i2c1, ADDR, &reg, 1, true);
    if (w < 0) {
        current_lux = -1.0f; // I2C write error
    }

    int r = i2c_read_blocking(i2c1, ADDR, data, 2, false);
    if (r != 2) {
        current_lux = -2.0f; // I2C read error
    }

    // data[0] = LSB, data[1] = MSB -> 16-bit ALS value
    uint16_t raw = (uint16_t)data[1] << 8 | data[0];

    // lux conversion: lux = raw * 0.012
    float lux = raw * 0.012f;
    current_lux = lux;

    // if(current_lux < lux_threshold){
    //     my_pwm_init(false, true); //clockwise
    // }
    // else{
    //     my_pwm_init(false, false); //counter clockwise
    // }
    uint target = timer_hw->timerawl + poll_rate_us;
    timer_hw->alarm[0] = target;

    uint8_t ps_reg = 0x08; // ALS_DATA register
    uint8_t ps_data[2];

    i2c_write_blocking(i2c1, ADDR, &ps_reg, 1, true);
    i2c_read_blocking(i2c1, ADDR, ps_data, 2, false);
    uint16_t ps_raw = (uint16_t)ps_data[1] << 8 | ps_data[0];

    printf("PS Value: %d\n", ps_raw);

}

void light_poll(){
    hw_set_bits(&timer_hw->inte, 1u << 0);
    irq_set_exclusive_handler(TIMER0_IRQ_0, read_lux);
    irq_set_enabled(TIMER0_IRQ_0, true);
    uint target = timer_hw->timerawl + poll_rate_us;
    timer_hw->alarm[0] = target;
    printf("Light poll called\n"); 
}