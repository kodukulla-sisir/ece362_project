#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

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
#include "pwm.h"


//////////////////////////////////////////////////////////////////////////////

// Make sure to set your pins if you are using this on your own breadboard.
// For the Platform Test Board, these are the correct pin numbers.
extern const int SPI_7SEG_SCK; // = 14;
extern const int SPI_7SEG_CSn; // = 13;
extern const int SPI_7SEG_TX;//  = 15;

// NOT NEEDED since we are not using LCD/OLED in this practical.
// But it needs to be defined to avoid compiler errors.
extern const int SPI_DISP_SCK; // = -1;
extern const int SPI_DISP_CSn; // = -1;
extern const int SPI_DISP_TX; // = -1;
extern const int MTR_IN1; //= 30; // A+
extern const int MTR_IN2; //= 29; // A-
extern const int MTR_IN3; //= 25; // B+
extern const int MTR_IN4; //= 26; // B-
extern const int ALL_CTRL_PINS; // = (1u << MTR_IN1) | (1u << MTR_IN2) | (1u << MTR_IN3) | (1u << MTR_IN4) ;


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


void my_pwm_init(bool dir) {

    // Top = 49,999
    // Clock Div = 20
    // Frequency out = 150 Hz
    //PWM initlization for motor pins
    gpio_set_function_masked(ALL_CTRL_PINS,GPIO_FUNC_PWM);
    uint16_t APS = pwm_gpio_to_slice_num(MTR_IN1);
    uint16_t AMS = pwm_gpio_to_slice_num(MTR_IN2);
    uint16_t BPS = pwm_gpio_to_slice_num(MTR_IN3);
    uint16_t BMS = pwm_gpio_to_slice_num(MTR_IN4);

    // Set clock values then enable at the same time, to sync.
    uint16_t clkdiv = 20;
    pwm_set_clkdiv(APS,clkdiv);
    pwm_set_clkdiv(AMS,clkdiv);
    pwm_set_clkdiv(BPS,clkdiv);
    pwm_set_clkdiv(BMS,clkdiv);

    //sets the top value for the counter
    uint16_t period = 49999u;
    pwm_hw->slice[APS].top = period;
    pwm_hw->slice[AMS].top = period;
    pwm_hw->slice[BPS].top = period;
    pwm_hw->slice[BMS].top = period;

    //sets the duty cycle to 50% 
    uint16_t duty_cyc = (period + 1) / 2;
    pwm_hw->slice[APS].cc = duty_cyc | (duty_cyc << 16);
    pwm_hw->slice[AMS].cc = duty_cyc | (duty_cyc << 16);
    pwm_hw->slice[BPS].cc = duty_cyc | (duty_cyc << 16);
    pwm_hw->slice[BMS].cc = duty_cyc | (duty_cyc << 16);

    uint16_t ap; //counter for APS (A+)
    uint16_t am; //counter for AMS (A-)
    uint16_t bp; //counter for BPS (B+)
    uint16_t bm; //counter for BMS (B-)

    // Set counters, A+ to 25,000, A- to 0, B+ to 37,500, and B- to 12,

    if(dir) //  clockwise (I think) if true
    {
        ap = (period + 1) / 2;
        am = 0;
        bp = ((period + 1) * 3) / 4;
        bm = (period + 1) / 4;
        }
        else {
        ap = (period + 1) / 2;
        am = 0;
        bp = (period + 1) / 4;
        bm = ((period + 1) * 3) / 4;
    }

    pwm_hw->slice[APS].ctr = ap;
    pwm_hw->slice[AMS].ctr = am;
    pwm_hw->slice[BPS].ctr = bp;
    pwm_hw->slice[BMS].ctr = bm;

    
    uint16_t chan = (1u << APS) | (1u << AMS) | (1u << BPS) | (1u << BMS); 
    pwm_hw->en = chan;
    if(dir){
        while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) >= (ap - am - 500)) && ((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) <= (ap - am + 500)))
        {
            pwm_hw->slice[AMS].csr = 1u | 1u << 6;
            //sleep_us(1);
        }
        while(((pwm_hw->slice[BPS].ctr - pwm_hw->slice[APS].ctr) >= (bp - ap - 500)) && ((pwm_hw->slice[BPS].ctr - pwm_hw->slice[APS].ctr) <= (bp - ap + 500)))
        {
            pwm_hw->slice[BPS].csr = 1u | 1u << 6;
            //sleep_us(1);
        }
        while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[BMS].ctr) >= (ap - bm - 500)) && ((pwm_hw->slice[APS].ctr - pwm_hw->slice[BMS].ctr) <= (ap - bm + 500)))
        {
            pwm_hw->slice[BMS].csr =1u |  1u << 6;
            //sleep_us(1);
        }
    }
    else {
        while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) >= (ap - am - 500)) && ((pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr) <= (ap - am + 500)))
        {
            pwm_hw->slice[AMS].csr = 1u | 1u << 6;
            //sleep_us(1);
        }
        while(((pwm_hw->slice[APS].ctr - pwm_hw->slice[BPS].ctr) >= (ap - bp - 500)) & ((pwm_hw->slice[APS].ctr - pwm_hw->slice[BPS].ctr) <= (ap - bp + 500)))
        {
            pwm_hw->slice[BPS].csr = 1u | 1u << 6;
            //sleep_us(1);
        }
        while(((pwm_hw->slice[BMS].ctr - pwm_hw->slice[APS].ctr) >= (bm - ap - 500)) && ((pwm_hw->slice[BMS].ctr - pwm_hw->slice[APS].ctr) <= (bm - ap + 500)))
        {
            pwm_hw->slice[BMS].csr =1u |  1u << 6;
            //sleep_us(1);
        }
    }
   
    
}
void motor_two()
{
    
gpio_set_function_masked((1u << 15u) | (1u << 16u),GPIO_FUNC_PWM);
    uint16_t APS = pwm_gpio_to_slice_num(15);
    uint16_t AMS = pwm_gpio_to_slice_num(16);

    // Set clock values then enable at the same time, to sync.
    uint16_t clkdiv = 20;
    pwm_set_clkdiv(APS,clkdiv);
    pwm_set_clkdiv(AMS,clkdiv);


    uint16_t period = 12500u;
    pwm_hw->slice[APS].top = period;


    uint16_t duty_cyc = (period + 1) / 2;
    pwm_hw->slice[APS].cc = duty_cyc | (duty_cyc << 16);

 //uint16_t chan = (1u << APS); //| (1u << AMS)
    pwm_hw->slice[APS].csr = 1u;
}


// void setState(int state)
// {
//     switch (state) {
//         case 0: 
//         gpio_clr_mask(ALL_CTRL_PINS);
//         gpio_set_mask(S0);
//         //gpio_xor_mask(S0 | S1)
//         break;
//         case 1:
//         gpio_clr_mask(ALL_CTRL_PINS);
//         gpio_set_mask(S1);
//         break;
//         case 2: 
//         gpio_clr_mask(ALL_CTRL_PINS);
//         gpio_set_mask(S2);
//         break;
//         case 3:
//         gpio_clr_mask(ALL_CTRL_PINS);
//         gpio_set_mask(S3);
//         break;
//         default:
//         printf("uh oh");
    
//     }

// }



///////////////////////////////////// 
// int main()
// {
//     my_gpio_init();
//     my_pwm_init(false);
//     motor_two();
//     //int state = 0;

//     // uint16_t APS = pwm_gpio_to_slice_num(MTR_IN1);
//     // uint16_t AMS = pwm_gpio_to_slice_num(MTR_IN2);
//     // uint16_t BPS = pwm_gpio_to_slice_num(MTR_IN3);
//     // uint16_t BMS = pwm_gpio_to_slice_num(MTR_IN4);
//     // while(1)
//     // {
//     //     setState(state);
//     //     state = ++state % 4;
//     //     sleep_ms(7);
//     // }
//     // while(1)
//     // {
//     //     setState(state);
//     //     if(state == 0)
//     //     {
//     //         state = 3;
//     //     }
//     //     else 
//     //     {
//     //         state--;
//     //     }
//     //     sleep_ms(7);
//     // }
// //u_int16_t a, b, c;
//     while(1)
//     {
//         // a = pwm_hw->slice[APS].ctr - pwm_hw->slice[AMS].ctr;
//         // c = pwm_hw->slice[APS].ctr - pwm_hw->slice[BPS].ctr;
//         // b = pwm_hw->slice[BPS].ctr - pwm_hw->slice[BMS].ctr;
//     }
//     for(;;);
//     return 0;
// }