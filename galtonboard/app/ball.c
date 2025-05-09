#include "ball.h"

// variáveis globais
// controle do estado das bolas em movimento e na fila de espera

int ball_falling_cnt = 0; // número atual de bolas em queda
int ball_queued_cnt = MAX_BALL_QUANT; // número de bolas aguardando para cair
ball_t balls_falling[MAX_BALL_QUANT]; // array com dados de posição e estado das bolas

// geração de direção aleatória
// determina a direção horizontal de uma bola com base em uma probabilidade
// parâmetros: chance (0-100) de retornar +1 (direita)
// retorno: +1 (direita) ou -1 (esquerda)

int get_random_direction(int chance) {
    uint64_t randValue = get_rand_64(); // obtém valor aleatório de 64 bits
    uint8_t randChance = (randValue % 100); // converte para valor entre 0-99
    if (randChance < chance) {
        return 1; // direita (probabilidade definida)
    } else {
        return -1; // esquerda
    }
}

// funções de desenho no display
// operações gráficas para renderizar e apagar bolas no display oled

// desenha uma bola como um quadrado vazio na posição (x, y)
void draw_ball(ssd1306_t *disp, int scale, int x, int y) {
    int y_offset = (disp->height - scale)/2; // centraliza verticalmente no display
    ssd1306_draw_empty_square(disp, x*scale, y*scale + y_offset, scale-1, scale-1);
}

// apaga uma bola da posição (x, y)
void erase_ball(ssd1306_t *disp, int scale, int x, int y) {
    int y_offset = (disp->height - scale)/2;
    ssd1306_clear_square(disp, x*scale, y*scale + y_offset, scale, scale);
}

// desenha todas as bolas em movimento
void draw_balls(ssd1306_t *disp, int scale) {
    for(int i = 0; i < ball_falling_cnt; i++) {
    draw_ball(disp, scale, balls_falling[i].x, balls_falling[i].y);
    }
}

// apaga todas as bolas em movimento
void erase_balls(ssd1306_t *disp, int scale) {
    for(int i = 0; i < ball_falling_cnt; i++) {
    erase_ball(disp, scale, balls_falling[i].x, balls_falling[i].y);
    }
}

// atualização de posições
// gerencia a física das bolas: movimento, saída do tabuleiro e reposição
// retorno: posição de saída da bola (-1 se nenhuma saiu)

int updt_balls_pos(int n_lines) {
    static bool start_occupied = false; // bloqueia novas bolas temporariamente
    static int start_occupied_tick_cnt = 0; // contador de ticks para desbloqueio
    int exit_pos = -1; // posição de saída detectada
    
    // verifica se a primeira bola saiu do tabuleiro  
    if(balls_falling[0].x >= 2*n_lines) {  
        ball_queued_cnt++;                         // recoloca a bola na fila  
        ball_falling_cnt--;                        // reduz contador de bolas ativas  
        exit_pos = (balls_falling[0].y + n_lines)/2;  // calcula posição de saída  
        // reorganiza o array removendo a bola que saiu  
        for(int i = 0; i < ball_falling_cnt; i++) {  
            balls_falling[i] = balls_falling[i+1];  
        }  
    }  
    
    // atualiza posição de todas as bolas ativas  
    for(int i = 0; i < ball_falling_cnt; i++) {  
        // movimento horizontal apenas em posições pares (passos alternados)  
        if(balls_falling[i].x % 2 == 0 && !balls_falling[i].force_fall) {  
            balls_falling[i].y += get_random_direction(50);  // move aleatoriamente  
            balls_falling[i].force_fall = true;              // bloqueia movimento no próximo passo  
        } else {  
            balls_falling[i].x += 1;          // movimento vertical para baixo  
            balls_falling[i].force_fall = false;  // libera movimento horizontal  
        }  
    }  
    
    // temporizador para evitar sobreposição no início  
    if(start_occupied) {  
        start_occupied_tick_cnt++;  
        if(start_occupied_tick_cnt > 4) {  
            start_occupied = false;  // desbloqueia após 4 atualizações  
        }  
    }  
    
    // adiciona nova bola se houver espaço e bolas na fila  
    if(!start_occupied && ball_queued_cnt > 0 && ball_falling_cnt < MAX_BALL_QUANT) {  
        balls_falling[ball_falling_cnt].x = 0;         // posição inicial x  
        balls_falling[ball_falling_cnt].y = 0;         // posição inicial y  
        balls_falling[ball_falling_cnt].force_fall = false;  
        ball_falling_cnt++;                            // incrementa contador  
        start_occupied = true;                         // bloqueia novas bolas  
        start_occupied_tick_cnt = 0;  
        ball_queued_cnt--;                             // decrementa fila  
    }  
    
    return exit_pos;  // retorna posição de saída ou -1  
    
}