#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "rotary_enc.pio.h"
#include "chardisp.h"
#include "lightsensor.h"
#include "pwm.h"

const int ADDR = 0x51;
const int LOWER_LUX = 50;
const int HIGH_LUX = 400;
const int LOW_THRESH = LOWER_LUX / 0.012;
const int HIGH_THRESH = HIGH_LUX / 0.012;
const int INT_PIN = 16;
const int SPI_7SEG_SCK = 14;
const int SPI_7SEG_CSn = 13;
const int SPI_7SEG_TX = 15;

// NOT NEEDED since we are not using LCD/OLED in this practical.
// But it needs to be defined to avoid compiler errors.
const int SPI_DISP_SCK = 34;
const int SPI_DISP_CSn = 33;
const int SPI_DISP_TX = 35;

const int MTR_IN1 = 30; // A+
const int MTR_IN2 = 29; // A-
const int MTR_IN3 = 25; // B+
const int MTR_IN4 = 26; // B-

// PIO constants 
const int A_PIN = 2;
const int B_PIN = 3; 
int32_t position = 0;
static uint8_t last_state = 0xFF; 
PIO pio = pio0;
uint sm = 0;  
const int full_pos_threshold = 100; // Number of detents for full angle 

const int ALL_CTRL_PINS = (1u << MTR_IN1) | (1u << MTR_IN2) | (1u << MTR_IN3) | (1u << MTR_IN4);

const int lux_threshold = 300; // temp lux val
float curr_lux = 0.0f;
const int spi_poll = 10000000; // 10 seconds 


void pio_position(void){
    uint32_t raw_data = pio_sm_get_blocking(pio, sm); 
    uint8_t state = raw_data & 0b11;
    uint8_t B = (state >> 1) & 1; 

    if (last_state == 0xFF) {
        last_state = state;
        return; 
    }
    if(B == 0){
        position++;// clockwise
    }else if(B == 1){
        position--; // Counter
    }
    last_state = state; 
    // printf("state : %u\n", state); 
    // printf("position : %ld\n", position);
    // printf("B : %ld\n", B);
} 

// void display_lux(char* str, int temp){
//     char temp_buffer[64]; 
//     snprintf(temp_buffer, sizeof(temp_buffer), "%s: %d", str, temp);
//     cd_display1(temp_buffer);
// }

void display_lux(){
    char temp_buffer[64]; 
    snprintf(temp_buffer, sizeof(temp_buffer), "%s: %d", "Lux Level", curr_lux);
    cd_display1(temp_buffer);
}


void spi_poll(){
    hw_set_bits(&timer_hw->inte, 1u << 1);
    irq_set_exclusive_handler(TIMER0_IRQ_1, display_lux);
    irq_set_enabled(TIMER0_IRQ_1, true);
    uint target = timer_hw->timerawl + spi_poll;
    timer_hw->alarm[1] = target;
}

int main()
{   

    stdio_init_all();
    light_init();
    light_poll();
    init_chardisp_pins(); 
    cd_init(); 

    display_temp("The temp is ", 23); 

    // Setting irq handler for PIO 
    uint offset = pio_add_program(pio, &rotary_enc_program);
    rotary_enc_program_init(pio, sm, offset, A_PIN, B_PIN);
    irq_set_exclusive_handler(PIO0_IRQ_0, pio_position);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio_set_irq0_source_enabled(pio, pis_sm0_rx_fifo_not_empty, true);

    if(curr_lux < lux_threshold){
        my_pwm_init(true); //clockwise
    }
    else{
        my_pwm_init(false); //counter clockwise
    }

    // Rotary encoder for shades angle 
    // Check if 
    if(position >= full_pos_threshold){
        // Stop moving 
    } else {
        // move
    }

    for(;;)
    {
        //tight_loop_contents();

    }

}

//////////////////////////////////////////////////////////////////////////////
