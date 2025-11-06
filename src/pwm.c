#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
//#include "support.h"

// Base library headers ncluded for your convenience.
// ** You may have to add more depending on your practical. **
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "pico/rand.h"



//////////////////////////////////////////////////////////////////////////////

// Make sure to set your pins if you are using this on your own breadboard.
// For the Platform Test Board, these are the correct pin numbers.
const int SPI_7SEG_SCK = 14;
const int SPI_7SEG_CSn = 13;
const int SPI_7SEG_TX = 15;

// NOT NEEDED since we are not using LCD/OLED in this practical.
// But it needs to be defined to avoid compiler errors.
const int SPI_DISP_SCK = -1;
const int SPI_DISP_CSn = -1;
const int SPI_DISP_TX = -1;

const int MTR_IN1 = 15; // A+
const int MTR_IN2 = 16; // A-
const int MTR_IN3 = 12; // B+
const int MTR_IN4 = 11; // B-

const int ALL_CTRL_PINS = (1u << MTR_IN1) | (1u << MTR_IN2) | (1u << MTR_IN3) | (1u << MTR_IN4) ;

const int S0 = (1u << MTR_IN1) | (1u << MTR_IN3);
const int S1 = (1u << MTR_IN2) | (1u << MTR_IN3);
const int S2 = (1u << MTR_IN2) | (1u << MTR_IN4);
const int S3 = (1u << MTR_IN1) | (1u << MTR_IN4);

//////////////////////////////////////////////
// Step 1

//////////////////////////////////////////////
//// Step 2. 

uint16_t __attribute__((aligned(16))) message[8] = {
    (0 << 8) | 0x3F,    // seven-segment value of 0
    (1 << 8) | 0x06,    // seven-segment value of 1
    (2 << 8) | 0x5B,    // seven-segment value of 2
    (3 << 8) | 0x4F,    // seven-segment value of 3
    (4 << 8) | 0x66,    // seven-segment value of 4
    (5 << 8) | 0x6D,    // seven-segment value of 5
    (6 << 8) | 0x7D,    // seven-segment value of 6
    (7 << 8) | 0x07,    // seven-segment value of 7
};
// 7-segment display font mapping
extern char font[];

/////////////////////////////////////
void my_gpio_init() {

gpio_init_mask(ALL_CTRL_PINS);
gpio_set_dir_out_masked(ALL_CTRL_PINS);

}


void my_pwm_init() {

    // Top = 49,999
    // Clock Div = 20
    // Frequency out = 150 Hz
    gpio_set_function_masked(ALL_CTRL_PINS,GPIO_FUNC_PWM);
    uint16_t APS = pwm_gpio_to_slice_num(MTR_IN1);
    uint16_t AMS = pwm_gpio_to_slice_num(MTR_IN2);
    uint16_t BPS = pwm_gpio_to_slice_num(MTR_IN3);
    uint16_t BMS = pwm_gpio_to_slice_num(MTR_IN4);
    // Set clock values then enable at the same time, to sync.
    pwm_set_clkdiv(APS,20);
    pwm_set_clkdiv(AMS,20);
    pwm_set_clkdiv(BPS,20);
    pwm_set_clkdiv(BMS,20);

    pwm_hw->slice[APS].top = 49999u;
    pwm_hw->slice[AMS].top = 49999u;
    pwm_hw->slice[BPS].top = 49999u;
    pwm_hw->slice[BMS].top = 49999u;

    pwm_hw->slice[APS].cc = 25000u | (25000u << 16);
    pwm_hw->slice[AMS].cc = 25000u | (25000u << 16);
    pwm_hw->slice[BPS].cc = 25000u | (25000u << 16);
    pwm_hw->slice[BMS].cc = 25000u | (25000u << 16);

    // Set counters, A+ to 25,000, A- to 0, B+ to 37,500, and B- to 12,500
    pwm_hw->slice[APS].ctr = 25000u;
    pwm_hw->slice[AMS].ctr = 0u;
    pwm_hw->slice[BPS].ctr = 37500u;
    pwm_hw->slice[BMS].ctr = 12500u;

    uint16_t chan = (1u << APS) | (1u << AMS) | (1u << BPS) | (1u << BMS); 
    pwm_hw->en = chan;

    while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) >= 24500u) & ((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) <= 25500))
    {
        pwm_hw->slice[AMS].csr = 1u << 6;
        sleep_us(1);
    }
      while(((pwm_hw->slice[BPS].ctr - pwm_hw->slice[APS].ctr) >= 12450u) & ((pwm_hw->slice[BPS].ctr - pwm_hw->slice[APS].ctr) <= 12550))
    {
        pwm_hw->slice[BPS].csr = 1u << 6;
        sleep_us(1);
    }
      while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[BMS].ctr) >= 12450u) & ((pwm_hw->slice[APS].ctr - pwm_hw->slice[BMS].ctr) <= 12550))
    {
        pwm_hw->slice[BMS].csr = 1u << 6;
        sleep_us(1);
    }
    
}


void setState(int state)
{
    switch (state) {
        case 0: 
        gpio_clr_mask(ALL_CTRL_PINS);
        gpio_set_mask(S0);
        //gpio_xor_mask(S0 | S1)
        break;
        case 1:
        gpio_clr_mask(ALL_CTRL_PINS);
        gpio_set_mask(S1);
        break;
        case 2: 
        gpio_clr_mask(ALL_CTRL_PINS);
        gpio_set_mask(S2);
        break;
        case 3:
        gpio_clr_mask(ALL_CTRL_PINS);
        gpio_set_mask(S3);
        break;
        default:
        printf("uh oh");
    
    }

}



///////////////////////////////////// 
int main()
{
    my_gpio_init();
    my_pwm_init();
    int state = 0;

    uint16_t APS = pwm_gpio_to_slice_num(MTR_IN1);
    uint16_t AMS = pwm_gpio_to_slice_num(MTR_IN2);
    uint16_t BPS = pwm_gpio_to_slice_num(MTR_IN3);
    uint16_t BMS = pwm_gpio_to_slice_num(MTR_IN4);
    // while(1)
    // {
    //     setState(state);
    //     state = ++state % 4;
    //     sleep_ms(7);
    // }
    // while(1)
    // {
    //     setState(state);
    //     if(state == 0)
    //     {
    //         state = 3;
    //     }
    //     else 
    //     {
    //         state--;
    //     }
    //     sleep_ms(7);
    // }
u_int16_t a, b, c;
    while(1)
    {
        a = pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr;
        c = pwm_hw->slice[APS].ctr - pwm_hw->slice[BPS].ctr;
        b = pwm_hw->slice[BPS].ctr - pwm_hw->slice[BMS].ctr;
    }
    for(;;);
    return 0;
}