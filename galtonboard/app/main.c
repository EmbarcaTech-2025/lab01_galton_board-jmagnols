#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "pico/stdlib.h"
#include "ssd1306.h"
#include "joystick.h"

#include "ball.h"
#include "histogram.h"

//  config geral 
// define parâmetros gerais do sistema como thresholds do joystick, limites de fps e frequências de atualização

#define JOYSTICK_N_BINS 10 // número de divisões angulares para detectar movimento do joystick
#define JOYSTICK_EXTEND_LIM 0.9f // limite radial para considerar o joystick estendido
#define JOYSTICK_RETRACT_LIM 0.8f // limite radial para considerar o joystick retraído

#define MAX_FPS 60 // taxa máxima de atualização do display (frames por segundo)

#define MAX_SIMULATION_UPDATE_FREQUENCY MAX_FPS*0.7 // frequência máxima de atualização da simulação
#define MIN_SIMULATION_UPDATE_FREQUENCY 1 // frequência mínima de atualização da simulação

// configuração de pinos
// mapeamento dos pinos físicos para componentes específicos do hardware

// configuração i2c para o display oled
#define I2C_PORT i2c1 // porta i2c utilizada
#define I2C_SDA 14 // pino de dados (SDA)
#define I2C_SCL 15 // pino de clock (SCL)

// pinos do joystick analógico
#define JOY_X_PIN 26 // pino do eixo x do joystick
#define JOY_Y_PIN 27 // pino do eixo y do joystick

// pino do buzzer para feedback sonoro
#define BUZZER_PIN 21

// funções do display oled
// desenha a estrutura do tabuleiro onde as bolas irão cair
// parâmetros: disp (referência do display), scale (escala dos elementos), n_lines (número de linhas)

void draw_board(ssd1306_t *disp, int scale, int n_lines) {
    int x_offset = scale; // deslocamento horizontal inicial
    int y_offset = (disp->height - scale)/2; // centraliza verticalmente

    // desenha linhas do tabuleiro em formato triangular
    for(int line = 0; line < n_lines; line++) {
        int n_pins = line + 1;  // número de pinos por linha aumenta progressivamente
        for(int column = 0; column < n_pins; column++) {
            // calcula posição de cada pino
            int x = x_offset + 2*scale*line;
            int y = y_offset - scale*line + 2*scale*column;
            ssd1306_draw_square(disp, x, y, scale, scale);  // desenha quadrado representando um pino
        }
        n_pins++;
    }
    ssd1306_show(disp);  // atualiza o display com as mudanças
}

// callback de atualização da simulação
// gerencia o loop principal da simulação: movimentação de bolas, histograma e temporização

ssd1306_t disp; // instância do display
volatile bool redraw = false; // flag para indicar necessidade de redesenhar o display
int scale = 1; // escala dos elementos gráficos
int n_lines = 31; // número de linhas no tabuleiro
int n_updates = 1; // número de atualizações de física por ciclo

int64_t simulation_delay_tick_us = 1<<19; // intervalo inicial entre atualizações (em microssegundos)

// função chamada periodicamente pelo timer para atualizar a simulação
bool simulation_tick_callback(repeating_timer_t *rt) {
    erase_balls(&disp, scale); // apaga bolas da posição anterior
    // executa múltiplas atualizações de posição conforme configurado
    for (int i = 0; i < n_updates; i++) {
        int exit_position = updt_balls_pos(n_lines);  // calcula nova posição das bolas

        if (exit_position != -1) {
            addto_hist(exit_position);  // atualiza histograma com posição de saída
        }
    }

    draw_balls(&disp, scale);  // redesenhia bolas nas novas posições
    draw_hist(&disp);  // atualiza o histograma no display

    redraw = true;  // solicita atualização do display
    rt->delay_us = simulation_delay_tick_us;  // ajusta intervalo para próximo ciclo

    return true;  // mantém o timer ativo
}

//  controle de velocidade da simulação
// ajusta a frequência de atualização com base no input do joystick

bool update_simulation_frequency(int update) {
    static float current_simulation_frequency = 2; // frequência atual em Hz
    bool changed = false;
    
    // lógica para aumentar velocidade
    if(update == 1) {
        if(current_simulation_frequency < MAX_SIMULATION_UPDATE_FREQUENCY) {
            current_simulation_frequency *= 1.3;  // aumento exponencial
            changed = true;
        }
        else if(n_updates < 4) { 
            n_updates++;  // incrementa atualizações por ciclo
            changed = true;
        }
    }
    // lógica para diminuir velocidade
    else if(update == -1) {
        if(n_updates > 1) {
            n_updates--;
            changed = true;
        }
        else if(current_simulation_frequency > MIN_SIMULATION_UPDATE_FREQUENCY){
            current_simulation_frequency *= 0.7;  // redução exponencial
            changed = true;
        }
    }
    
    // converte frequência para intervalo de tempo
    simulation_delay_tick_us = 1000000.0 / current_simulation_frequency;
    
    return changed;  // indica se houve mudança
}

// lógica do joystick
// converte movimento do joystick em comandos para ajustar a simulação

// callback para desligar o buzzer após um curto período
int64_t buzzer_off_callback(alarm_id_t id, void *user_data) {
    gpio_put(BUZZER_PIN, false);
    return 0; // alarme único, não se repete
    }
    
    // processa o estado do joystick e retorna ajuste de velocidade (+1, -1 ou 0)
    int updt_joystick() {
    static bool extended = false; // indica se joystick está estendido
    static int current_bin = -1, last_bin = -1; // posições angulares atual e anterior
    
    float r, theta;
    int update = 0;
    
    getRA_joystick(&r, &theta);  // obtém coordenadas polares
    
    // verifica transições de estado (extendido/retraído)
    if (r > JOYSTICK_EXTEND_LIM) {
        extended = true;
    } else if (r < JOYSTICK_RETRACT_LIM) {
        extended = false;
        last_bin = -1;  // reseta estado anterior
    }
    
    if (extended) {
        if (theta < 0) theta += 2 * M_PI;  // normaliza ângulo para 0-2π
    
        // calcula bin atual baseado na posição angular
        current_bin = (int)(theta * JOYSTICK_N_BINS / (2 * M_PI)) % JOYSTICK_N_BINS;
    
        // detecta movimento circular (sentido horário ou anti-horário)
        if (last_bin != -1) {
            int diff = (current_bin - last_bin + JOYSTICK_N_BINS) % JOYSTICK_N_BINS;
    
            if (diff == 1) update = -1;  // movimento anti-horário
            else if (diff == JOYSTICK_N_BINS - 1) update = +1;  // movimento horário
        }
    
        last_bin = current_bin;  // armazena posição para próxima iteração
    }
    
    return update;  // -1: reduzir velocidade, +1: aumentar, 0: sem mudança
    
}

// função principal
// inicializa hardware e executa loop principal com lógica de controle

int main() {
    stdio_init_all(); // inicializa comunicação serial

    // configura display OLED
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT, I2C_SDA, I2C_SCL);

    init_joystick(JOY_X_PIN, JOY_Y_PIN);  // inicializa leitura do joystick

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, true);  // configura buzzer como saída

    ssd1306_clear(&disp);
    draw_board(&disp, scale, n_lines);  // desenha estrutura inicial

    init_hist(n_lines + 1);  // inicializa histograma com n_lines+1 posições

    // configura timer para atualização periódica da simulação
    repeating_timer_t timer;
    add_repeating_timer_ms(1000, simulation_tick_callback, NULL, &timer);

    absolute_time_t last_redraw_time = get_absolute_time();  // controle de fps

    // loop principal
    while (true) {
        absolute_time_t now = get_absolute_time();

        int update = updt_joystick();  // obtém ajuste de velocidade do joystick

        // aplica mudança na frequência e aciona buzzer se necessário
        bool frequency_changed = update_simulation_frequency(update);
        if(frequency_changed) {
            gpio_put(BUZZER_PIN, true);
            add_alarm_in_ms(2, buzzer_off_callback, NULL, false);  // bip curto
        }

        // atualiza display respeitando o limite de fps
        if(redraw && absolute_time_diff_us(last_redraw_time, now) > 1000000.0 / MAX_FPS) {
            redraw = false;
            last_redraw_time = now;
            ssd1306_show(&disp);  // renderiza buffer no display
        }

        sleep_ms(5);  // reduz consumo de CPU
    }
}