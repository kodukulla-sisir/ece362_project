#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "chardisp.h"

// Make sure to set these in main.c
extern const int SPI_DISP_SCK; extern const int SPI_DISP_CSn; extern const int SPI_DISP_TX;

/***************************************************************** */

// "chardisp" stands for character display, which can be an LCD or OLED
void init_chardisp_pins() {
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);

    spi_init(spi0, 10000); 
    spi_set_format(spi0, 9, 0, 0, SPI_MSB_FIRST); 
}

void send_spi_cmd(spi_inst_t* spi, uint16_t value) {
    while(spi_is_busy(spi) == true){
        // Wait for it 
    }
    spi_write16_blocking(spi0, &value, 1); 
}

void send_spi_data(spi_inst_t* spi, uint16_t value) {
    uint16_t final_value = value | 0x100; 
    send_spi_cmd(spi, final_value);
}

void cd_init() {
    sleep_ms(1); 
    send_spi_cmd(spi0, 0b101100); 
    sleep_ms(1); 
    send_spi_cmd(spi0, 0b1100); 
    sleep_ms(1); 
    send_spi_cmd(spi0, 0b1); 
    sleep_ms(2); 
    send_spi_cmd(spi0, 0b110); 
    sleep_ms(2); 
}

void cd_display1(const char *str) {
    send_spi_cmd(spi0, 0b10); 
    while(*str != '\0'){
        send_spi_data(spi0, *str);
        str += 1; 
    }
}

void cd_display2(const char *str) {
    send_spi_cmd(spi0, 0b11000000); 
    while(*str != '\0'){
        send_spi_data(spi0, *str);
        str += 1; 
    }
}

/***************************************************************** */