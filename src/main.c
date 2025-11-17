#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

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
const int SPI_DISP_SCK = -1;
const int SPI_DISP_CSn = -1;
const int SPI_DISP_TX = -1;

const int MTR_IN1 = 30; // A+
const int MTR_IN2 = 29; // A-
const int MTR_IN3 = 25; // B+
const int MTR_IN4 = 26; // B-

const int ALL_CTRL_PINS = (1u << MTR_IN1) | (1u << MTR_IN2) | (1u << MTR_IN3) | (1u << MTR_IN4);


light_init();

int main()
{
    stdio_init_all();
    light_init();

    for(;;)
    {
        //tight_loop_contents();
    }
}

//////////////////////////////////////////////////////////////////////////////
