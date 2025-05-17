#include <stdio.h>        // Funções de entrada e saída padrão
#include "pico/stdlib.h"  // Funcionalidades básicas do RP2040
#include "pico/bootrom.h" // Para entrar no modo bootsel ao pressionar o botão B
#include "hardware/pwm.h" // Biblioteca para utilizar PWM

// -------------------------------- DEFINES - I --------------------------------

// Configurações dos botões
#define BUTTON_A 5 // Pino do botão A
#define BUTTON_B 6 // Pino do botão B

#define RED_LED 13      // Pino do LED vermelho
#define BLUE_LED 12     // Pino do LED azul
#define GREEN_LED 11     // Pino do LED verde

// -------------------------------- DEFINES - F --------------------------------



// -------------------------------- VARS - I --------------------------------

static volatile uint32_t last_time = 0; // Armazena o último tempo registrado nas interrupções

volatile int pwm_type = 0; // Indica o tipo de sinal pwm que deve ser enviado
volatile bool pwm_flag = false; // Flag indicando mudança no sinal PWM

// -------------------------------- VARS - F --------------------------------



// -------------------------------- FUNCTIONS - I --------------------------------

uint pwm_start(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice= pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 255);
    pwm_set_wrap(slice, 65535);
    pwm_set_gpio_level(gpio, 0);
    pwm_set_enabled(slice, true);

    return slice;

}

// 1kHz 50%
uint pwm_signal_0(uint gpio) {
    uint slice= pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 40.f);
    pwm_set_wrap(slice, 3124);
    pwm_set_gpio_level(gpio, 1562);
    pwm_set_enabled(slice, true);

    return slice;

}

// 8 Hz 20%
uint pwm_signal_1(uint gpio) {
    uint slice= pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 238.4186f);
    pwm_set_wrap(slice, 65535);
    pwm_set_gpio_level(gpio, 13107);
    pwm_set_enabled(slice, true);

    return slice;

}

// 160 kHz 75%
uint pwm_signal_2(uint gpio) {
    uint slice= pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 156.25f);
    pwm_set_wrap(slice, 4);
    pwm_set_gpio_level(gpio, 3);
    pwm_set_enabled(slice, true);

    return slice;

}

// Inicializa o botão B
void init_buttons() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

// Callback para tratar o botão B (reset para modo BOOTSEL)
void gpio_irq_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual em ms

    // Debounce de 500 ms
    if( (current_time - last_time) > 500 ) {
        if(gpio == BUTTON_A) {
            pwm_flag = true;
        }
        if(gpio == BUTTON_B) {
            reset_usb_boot(0, 0);
        }
        last_time = current_time;
    }
}

// -------------------------------- FUNCTIONS - F --------------------------------



int main()
{
    uint used_pin = 16;

    stdio_init_all();

    init_buttons();

    pwm_start(used_pin);
    pwm_start(RED_LED);
    pwm_start(BLUE_LED);
    pwm_start(GREEN_LED);

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback); // Configura a interrupção para o botão A
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback); // Configura a interrupção para o botão B

    while (true) {

        if(pwm_flag) {
            if(pwm_type == 0) {
                pwm_signal_0(used_pin);

                pwm_signal_0(RED_LED);
                pwm_set_gpio_level(BLUE_LED, 0);
                pwm_set_gpio_level(GREEN_LED, 0);
                printf("SINAL 0\n");
                pwm_type++;
            }else
            if(pwm_type == 1) {
                pwm_signal_1(used_pin);

                pwm_signal_1(BLUE_LED);
                pwm_set_gpio_level(RED_LED, 0);
                pwm_set_gpio_level(GREEN_LED, 0);
                printf("SINAL 1\n");
                pwm_type++;
            }else
            if(pwm_type == 2) {
                pwm_signal_2(used_pin);

                pwm_signal_2(GREEN_LED);
                pwm_set_gpio_level(RED_LED, 0);
                pwm_set_gpio_level(BLUE_LED, 0);
                printf("SINAL 2\n");
                pwm_type = 0;
            }
            pwm_flag = false;
        }

    }
}
