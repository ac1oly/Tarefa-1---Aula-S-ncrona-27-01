#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "tarefa02.pio.h"

// Definições dos LEDs
#define LEDG 11
#define LEDB 12
#define LEDR 13

// Definições dos botões
#define BUTTON_A 5
#define BUTTON_B 6

// Definições da matriz de LED
#define OUT_PIN 7
#define NUM_PIXELS 25

// Tempo para debounce
#define DEBOUNCE 50

// Brilho dos LEDs (0 a 255)
#define BRILHO 2

// Variável para o contador
static volatile int cont = 0;

// Inicialização do periférico PIO
PIO pio = pio0;
int sm = 0;

// Array RGB dos LEDs
uint32_t leds[NUM_PIXELS];

// Definição da matriz dos números de 0 a 9
const bool NUMEROS[10][25] = {

    {// 0
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 1
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},
    {// 2
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 3
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 4
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 0, 0, 0, 1},
    {// 5
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 6
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 7
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 0, 0, 0},
    {// 8
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 9
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1}};

// FunÃ§Ã£o para criar cores RGB com ajuste de brilho
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
    return ((uint32_t)(g * brightness / 255) << 8) | ((uint32_t)(r * brightness / 255) << 16) | (uint32_t)(b * brightness / 255);
}

// Mapeamento da ordem dos LEDs conforme a matriz fÃ­sica
const int led_map[25] = {
    24, 23, 22, 21, 20,
    15, 16, 17, 18, 19,
    14, 13, 12, 11, 10,
    5, 6, 7, 8, 9,
    4, 3, 2, 1, 0};

// Atualiza a exibiÃ§Ã£o do nÃºmero nos LEDs
void update_display(int digit)
{
    for (int i = 0; i < 25; i++)
    {
        int mapped_index = led_map[i];
        leds[mapped_index] = NUMEROS[digit][i] ? urgb_u32(255, 0, 255, BRILHO) : 0;
    }

    for (int i = 0; i < 25; i++)
    {
        pio_sm_put_blocking(pio, sm, leds[i] << 8u);
    }
}

// Callback para debounce do botÃ£o
static int64_t debounce_callback(alarm_id_t id, void *user_data)
{
    uint gpio = (uint)user_data;
    if (gpio_get(gpio) == 0)
    {
        if (gpio == BUTTON_A)
        {
            cont = (cont + 1) % 10;
        }
        else if (gpio == BUTTON_B)
        {
            cont = (cont - 1 + 10) % 10;
        }
        update_display(cont);
    }
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
    return 0;
}

// InterrupÃ§Ã£o do botÃ£o
void button_isr(uint gpio, uint32_t events)
{
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
    add_alarm_in_ms(DEBOUNCE, debounce_callback, (void *)gpio, false);
}

// Timer para piscar o LED vermelho
bool blink_timer_callback(struct repeating_timer *t)
{
    gpio_put(LEDR, !gpio_get(LEDR));
    return true;
}

int main()
{
    stdio_init_all();

    // ConfiguraÃ§Ã£o dos LEDs RGB
    gpio_init(LEDR);
    gpio_set_dir(LEDR, GPIO_OUT);
    gpio_init(LEDG);
    gpio_set_dir(LEDG, GPIO_OUT);
    gpio_init(LEDB);
    gpio_set_dir(LEDB, GPIO_OUT);

    // ConfiguraÃ§Ã£o dos botÃµes
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicializa o WS2812 via PIO
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, OUT_PIN, 800000, false);

    // ConfiguraÃ§Ã£o das interrupÃ§Ãµes dos botÃµes
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_isr);

    // ConfiguraÃ§Ã£o do timer para piscar LED
    struct repeating_timer timer;
    add_repeating_timer_ms(100, blink_timer_callback, NULL, &timer);

    // Inicia o display mostrando "0"
    update_display(0);

    // Loop principal
    while (true)
    {
        tight_loop_contents();
    }

    return 0;
}