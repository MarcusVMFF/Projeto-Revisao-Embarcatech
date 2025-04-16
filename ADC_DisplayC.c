#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"  
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ADC_DisplayC.pio.h"
#include "hardware/timer.h"

//Definições do display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
//Definições da matriz 5x5
#define NUM_PIXELS 25
#define MATRIXPIO 7
//Definições buzzer e microfone
#define BUZZER_PIN 21
//Definições PWM
#define PWM_FREQ 1000        
#define CLOCK_DIV 125        
#define WRAP_VALUE 999 
//Definições do joystick, leds e botões
#define VRX_PIN 26  
#define VRY_PIN 27  
#define SW_PIN 22
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_PIN 11  
#define LED2_PIN 13  
#define LED3_PIN 12 
// Variaveis globais
volatile int cor_atual = 0;
volatile int freq = 500;
bool som_tocado = false;
int thickness = 1;
// Função de tocar o buzzer
void tocar_buzzer(int freq, int duration_ms)
    {
        uint32_t delay_us = 1000000 / (2 * freq);          
        uint32_t ciclos = (duration_ms * 1000) / delay_us; 

        for (uint32_t t = 0; t < ciclos; t++)
        {
            gpio_put(BUZZER_PIN, 1); 
            sleep_us(delay_us);
            gpio_put(BUZZER_PIN, 0); 
            sleep_us(delay_us);
        }
    }
    
// Quadros do painel de led 5x5
double desenho1[25] = {1, 1, 0, 1, 1,
                        1, 1, 0, 1, 1,
                        0, 0, 0, 0, 0,
                        1, 0, 0, 0, 1,
                        1, 1, 1, 1, 1,};
                        
double desenho2[25] = {0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0};

// Funcoes para funcionamento da matriz LED ter 3 cores diferentes
void imprimir_binario(int num) {
    int i;
    for (i = 31; i >= 0; i--) {
     (num & (1 << i)) ? printf("1") : printf("0");
    }
   }
   
   uint32_t matrix_rgb(double b, double r, double g)
   {
     unsigned char R, G, B;
     R = r * 255 * 0.125;
     G = g * 255 * 0.125;
     B = b * 255 * 0.125;
     return (G << 24) | (R << 16) | (B << 8);
   }
   
   void desenho_pioRED(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){
   
       for (int16_t i = 0; i < NUM_PIXELS; i++) {
           if (i%2==0)
           {
               valor_led = matrix_rgb(b=0, desenho[24-i], g=0.0);
               pio_sm_put_blocking(pio, sm, valor_led);
   
           }else{
               valor_led = matrix_rgb(b=0, desenho[24-i], g=0.0);
               pio_sm_put_blocking(pio, sm, valor_led);
           }
       }
       //imprimir_binario(valor_led);
   }
   void desenho_pioBLUE(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){
   
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (i%2==0)
        {
            valor_led = matrix_rgb(desenho[24-i], r=0, g=0.0);
            pio_sm_put_blocking(pio, sm, valor_led);

        }else{
            valor_led = matrix_rgb(desenho[24-i], r=0, g=0.0);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }
    //imprimir_binario(valor_led);
}
void desenho_pioGREEN(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){
   
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (i%2==0)
        {
            valor_led = matrix_rgb(b=0, r=0, desenho[24-i]);
            pio_sm_put_blocking(pio, sm, valor_led);

        }else{
            valor_led = matrix_rgb(b=0, r=0, desenho[24-i]);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }
    //imprimir_binario(valor_led);
}


// Definições de posições para o efeito dos LEDs
#define CENTRAL_POSITION 1970  
#define TOLERANCE 100          
#define MAX_PWM 4095           
#define MID_PWM (MAX_PWM / 2)  

// INICIAÇÃO PADRÃO PWM
bool ledpwm = true;
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

// Rotina de interrupção para o botão A e botão B com debouncing
uint32_t last_time_sw = 0;  
uint32_t last_time_btn = 0; 
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && (current_time - last_time_btn > 300000)) {
        last_time_btn = current_time; 
        som_tocado = true;
        cor_atual++;
        freq = freq + 200;
    }
    if (gpio == BUTTON_B) {
        last_time_btn = current_time;
        reset_usb_boot(0, 0);
    }
}

int main() {
    stdio_init_all();

    //Inicializa ADC
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    PIO pio = pio0;
    bool ok;
    ok = set_sys_clock_khz(128000, false);
    uint32_t valor_led;
    uint offset = pio_add_program(pio, &ADC_DisplayC_program);
    uint sm = pio_claim_unused_sm(pio, true);
    double r = 0.0, b = 0.0 , g = 0.0;

    //Inicialização matriz 5x5
    ADC_DisplayC_program_init(pio, sm, offset, MATRIXPIO);

    // Inicializações padrões do display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL); 
    ssd1306_t ssd; 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd); 
    bool cor = true;
    //Inicializa o GPIO
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN); 
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false); 
    gpio_init(LED2_PIN);
    gpio_set_dir(LED2_PIN, GPIO_OUT);
    gpio_put(LED2_PIN, false); 
    gpio_init(LED3_PIN);
    gpio_set_dir(LED3_PIN, GPIO_OUT);
    gpio_put(LED3_PIN, false);

    // Inicializa todos os pushbuttons
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); 
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    //Inicializa o buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Configura as interrupções para todos os botões
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);  
    gpio_set_irq_callback(&gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // PWM
    uint pwm_wrap = MAX_PWM;
    pwm_init_gpio(LED_PIN, pwm_wrap);
    pwm_init_gpio(LED2_PIN, pwm_wrap);
 
    
    uint32_t last_print_time = 0; 
    while (true) {
        adc_select_input(0);
        uint16_t vrx_value = adc_read();  
        adc_select_input(1);
        uint16_t vry_value = adc_read();
        int square_x = ((4095 - vrx_value) * 56) / 4095;  
        int square_y = (vry_value * 120) / 4095;   
        
        // Limita as coordenadas para evitar que o quadrado saia da tela
        if (square_y < 0) square_y = 0;
        if (square_y > 120) square_y = 120; 
        if (square_x < 0) square_x = 0;
        if (square_x > 56) square_x = 56;  
        ssd1306_fill(&ssd, !cor);
        for (int i = 0; i < thickness; i++) {
            ssd1306_rect(&ssd, 3 - i, 3 - i, 122 + 2 * i, 60 + 2 * i, cor, false);
        }

        // Desenha o quadrado na posição mapeada
        ssd1306_rect(&ssd, square_x, square_y, 8, 8, cor, true);

        ssd1306_send_data(&ssd);

        if (ledpwm) {
            if (vrx_value > (CENTRAL_POSITION - TOLERANCE) && vrx_value < (CENTRAL_POSITION + TOLERANCE)) {
                pwm_set_gpio_level(LED_PIN, 0);
            } else {
                int distance_from_center_red = abs(vrx_value - CENTRAL_POSITION);
                uint16_t mapped_value_red = (distance_from_center_red * MAX_PWM) / (MAX_PWM - MID_PWM);
                pwm_set_gpio_level(LED_PIN, mapped_value_red);
            }

            if (vry_value > (CENTRAL_POSITION - TOLERANCE) && vry_value < (CENTRAL_POSITION + TOLERANCE)) {
                pwm_set_gpio_level(LED2_PIN, 0);
            } else {
                int distance_from_center_blue = abs(vry_value - CENTRAL_POSITION);
                uint16_t mapped_value_blue = (distance_from_center_blue * MAX_PWM) / (MAX_PWM - MID_PWM);
                pwm_set_gpio_level(LED2_PIN, mapped_value_blue);
            }
        }
        if (som_tocado) {
            tocar_buzzer(freq, 150);
            som_tocado = false;
        }
        switch (cor_atual) {
            case 0:
                // Cor vermelha
                desenho_pioRED(desenho1, valor_led, pio, sm, r, g, b);
                break;
            case 1:
                // Cor azul
                desenho_pioBLUE(desenho1, valor_led, pio, sm, r, g, b);
                break;
            case 2:
                // Cor verde
                desenho_pioGREEN(desenho1, valor_led, pio, sm, r, g, b);
                break;
            case 3:
                // Cor "vazio"
                desenho_pioRED(desenho2, valor_led, pio, sm, r, g, b);
                break;
            default:
            //Reinicia
                cor_atual = 0;
                freq = 500;
                break;
        }
               
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(100);
        float duty_cycle = (vrx_value / 4095.0) * 100;  
        uint32_t current_time = to_ms_since_boot(get_absolute_time());  
        if (current_time - last_print_time >= 1000) {  
            printf("VRX: %u\n", vrx_value);
            printf("VRY: %u\n", vry_value); 
            printf("Duty Cycle LED: %.2f%%\n", duty_cycle); 
            last_print_time = current_time;  
        }
        sleep_ms(100);  
    }

    return 0;
}