#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

//////////////////////////////////////////////////////////////////////////////

// No autotest, but you might want to set this anyway.
const char* username = "skodukul";

// When testing basic TX/RX
#define STEP2
// When connecting UART to printf(), fgets()
// #define STEP3
// When testing UART IRQ for buffering
#define STEP4
// When testing PCS
// #define STEP5

//////////////////////////////////////////////////////////////////////////////

void init_uart() {
    gpio_set_function(0, UART_FUNCSEL_NUM(uart, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));
    uart_init(uart0, 115200);
    uart_set_format(uart0, 8, 1, 0);
    // fill in
}

#ifdef STEP2
int main() {
    init_uart();
    for (;;) {
        char buf[2];
        uart_read_blocking(uart0, (uint8_t*)buf, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0'; // Ensure null-termination
        uart_puts(uart0, "You said: ");
        uart_puts(uart0, buf);
        uart_puts(uart0, "\n");
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP3

// 3.3
int _read(__unused int handle, char *buffer, int length) {
    // Your code here to read from the UART and fill the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we read from the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to read, read 1 
    // character at a time from the UART until the buffer is 
    // filled or the length is reached. 
    for (int i = 0; i < length; i++) {
        char backspace = 8;
        uart_read_blocking(uart0, buffer, 1);

        if (buffer[i] == backspace){
            uart_putc(uart0, backspace);
            uart_putc(uart0, 32);
            uart_putc(uart0, backspace);
            i--;
        }
        else{
            uart_putc(uart0, buffer[i]);
        }
    }
}

int _write(__unused int handle, char *buffer, int length) {
    // Your code here to write to the UART from the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we write to the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to write, write 1
    // character at a time to the UART until the length is reached. 
    for (int i = 0; i < length; ++i) {
        while (!uart_is_writable(uart0))
            tight_loop_contents();
        uart_get_hw(uart0)->dr = *buffer++;
    }
}

int main()
{
    init_uart();

    // insert any setbuf lines below...
    /*
    setbuf(stdout, NULL);  // Disable buffering for stdout
    setbuf(stdin, NULL);   // Disable buffering for stdin
    for(;;) {
        putchar(getchar()); 
    }*/
    setbuf(stdout, NULL);  // Disable buffering for stdout
    setbuf(stdin, NULL);   // Disable buffering for stdin
    char name[8];
    int age = 0;
    for(;;) {
        printf("Enter your name and age: ");
        scanf("%s %d", name, &age);
        printf("Hello, %s! You are %d years old.\n", name, age);
        sleep_ms(100);  // in case the output loops and is too fast
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP4

#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// add this here so that compiler does not complain about implicit function
void uart_rx_handler();

void init_uart_irq() {
    uart_set_irqs_enabled(uart0, true, false);
    uart_set_fifo_enabled(uart0, false);
    irq_set_enabled(UART0_IRQ, true);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    // fill in.
}

void uart_rx_handler() {
    uart_get_hw(uart0)->icr = (0<<4);
    if (seridx == BUFSIZE){
        return;
    }
    char c = uart_get_hw(uart0) -> dr;
    if (c == 0x0A){
        newline_seen = 1;
    }
    if(c == 8 && (seridx > 0)){
        // char back[2] = {8,32,8};
        uart_write_blocking(uart0, "\b \b", 3);
        seridx--;
        serbuf[seridx] = '\0';
    }
    else{
        uart_write_blocking(uart0, (const uint8_t *)&c, 1);
        serbuf[seridx] = c;
        seridx++;

    }
    // fill in.
}

int _read(__unused int handle, char *buffer, int length) {
    while(newline_seen == 0){
        sleep_ms(5);
    }
    if (newline_seen > 0){
        newline_seen = 0;
    }
    for(int i = 0; i < length; i++){
        buffer[i] = serbuf[i];
    }
    seridx = 0;
    return(length);
    // fill in.
}

int _write(__unused int handle, char *buffer, int length) {
    // fill in.
    for (size_t i = 0; i < length; ++i) {
        while (!uart_is_writable(uart0))
            tight_loop_contents();
        uart_get_hw(uart0)->dr = *buffer++;
    }
}

int main() {
    init_uart();
init_uart_irq();

setbuf(stdout, NULL); // Disable buffering for stdout

char name[8];
int age = 0;
for(;;) {
    printf("Enter your name and age: ");
    scanf("%s %d", name, &age);
    // THIS IS IMPORTANT.
    fflush(stdin);
    printf("Hello, %s! You are %d years old.\r\n", name, age);
    sleep_ms(100);  // in case the output loops and is too fast
}
    // fill in.
}

#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP5

// Copy global variables, init_uart_irq, uart_rx_handler, _read, and _write from STEP4.
#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// add this here so that compiler does not complain about implicit function
void uart_rx_handler();

void init_uart_irq() {
    uart_set_irqs_enabled(uart0, true, false);
    uart_set_fifo_enabled(uart0, false);
    irq_set_enabled(UART0_IRQ, true);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    // fill in.
}

void uart_rx_handler() {
    uart_get_hw(uart0)->icr = (0<<4);
    if (seridx == BUFSIZE){
        return;
    }
    char c = uart_get_hw(uart0) -> dr;
    if (c == 0x0A){
        newline_seen = 1;
    }
    if(c == 8 && (seridx > 0)){
        // char back[2] = {8,32,8};
        uart_write_blocking(uart0, "\b \b", 3);
        seridx--;
        serbuf[seridx] = '\0';
    }
    else{
        uart_write_blocking(uart0, (const uint8_t *)&c, 1);
        serbuf[seridx] = c;
        seridx++;

    }
    // fill in.
}

int _read(__unused int handle, char *buffer, int length) {
    while(newline_seen == 0){
        sleep_ms(5);
    }
    if (newline_seen > 0){
        newline_seen = 0;
    }
    for(int i = 0; i < length; i++){
        buffer[i] = serbuf[i];
    }
    seridx = 0;
    return(length);
    // fill in.
}

int _write(__unused int handle, char *buffer, int length) {
    // fill in.
    for (size_t i = 0; i < length; ++i) {
        while (!uart_is_writable(uart0))
            tight_loop_contents();
        uart_get_hw(uart0)->dr = *buffer++;
    }
}
void cmd_gpio(int argc, char **argv) {
    // This is the main command handler for the "gpio" command.
    // It will call either cmd_gpio_out or cmd_gpio_set based on the arguments.
    
    // Ensure that argc is at least 2, otherwise print an example use case and return.

    // If the second argument is "out":
    //      Ensure that argc is exactly 3, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Set the pin to output using gpio_init and gpio_set_dir.
    //      Print a success message.
    
    // If the second argument is "set":
    //      Ensure that argc is exactly 4, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Check if the pin has been initialized as a GPIO output, if not, return.
    //      Convert the fourth argument to an integer value (0 or 1) using atoi.
    //      Check if the value is valid (0 or 1), otherwise print an error and return.
    //      Set the pin to the specified value using gpio_put.
    //      Print a success message.
    
    // Else, print an unknown command error.
    char * set = "set";
    char * out = "out";
    char * gpio = "gpio";

    if(!(strcmp(out, argv[1]))){
        if(argc != 3){
            printf("Error. Please refer to the given example:\ngpio out 22\ngpio set 22 1");
            }
            int num_gpio = atoi(argv[2]);
            if(!(num_gpio > 0 && num_gpio < 48)){
                printf("Invalid pin number: %d. Must be between 0 and 47.", num_gpio);
        }
        else{
            gpio_init(num_gpio);
            gpio_set_dir(num_gpio, 1);
            printf("Initialized pin %d as output.", atoi(argv[2]));
        }
    }
        if(!(strcmp(set,argv[1]))){
            int state = atoi(argv[3]);
            if(argc != 4){
                printf("Error. Please refer to the given example:\ngpio out 22\ngpio set 22 1");
            }
            int num_gpio = atoi(argv[2]);
            if(!(num_gpio > 0 && num_gpio < 48)){
                printf("Invalid pin number: %d. Must be between 0 and 47.", num_gpio);
            }
            if(!gpio_get_dir(num_gpio)){
                printf("Pin %d is not initialized as an output.", num_gpio);
            }
            else if(!(state == 0 || state == 1)){
                printf("Invalid state. Must be either 1 or 0.");
            }
            else{
                gpio_put(num_gpio, state);
            printf("Set pin %d to %d.", num_gpio, state);
            }
        }
        if (argc < 2){
            printf("Error. Please refer to the given example:\ngpio out 22\ngpio set 22 1");
        }
        else if((strcmp(argv[0], gpio)) && strcmp(argv[1], set) && strcmp(argv[2], out)){
            printf("\nUnknown Command.");
        }
    }
int main() {
    // See lab for instructions.
    init_uart();
    init_uart_irq();
    setbuf(stdout, NULL); 

    int argc=0;
    char *argv[10];
    char input[100];
    char space = 32;
    printf("%s's Peripheral Command Shell (PCS)\n", username);
    printf("Enter a command below.\n\n");
    for(;;){
        printf("\r\n>");
        fgets(input, sizeof(input), stdin);
        fflush(stdin);
        size_t index = strcspn(input, '\n');
        input[index] = 0;
        char *temp = strtok(input, " ");
        while(temp != NULL){
            argv[argc] = temp;
            temp = strtok(NULL, " ");
            argc++;
        }
        cmd_gpio(argc, argv);
        argc = 0;
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////
