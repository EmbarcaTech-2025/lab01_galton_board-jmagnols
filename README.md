
# Projetos de Sistemas Embarcados - EmbarcaTech 2025

Autor: João Magno Lourenço Soares

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, 9 de maio de 2025

# Simulação de Tábua de Galton no Raspberry Pi Pico

Este projeto implementa uma simulação interativa da Tábua de Galton (Galton Board) utilizando um microcontrolador Raspberry Pi Pico, um display OLED SSD1306 e um joystick analógico. Ele demonstra visualmente conceitos de movimento aleatório, probabilidade e a emergência da distribuição normal.

**Exemplo Visual de uma Tábua de Galton:**
[https://zerodha.com/varsity/chapter/volatility-normal-distribution/](https://zerodha.com/varsity/chapter/volatility-normal-distribution/)

## Conceitos de Física e Matemática Demonstrados

O projeto ilustra os seguintes conceitos fundamentais:

* **Movimento Aleatório:** As esferas (representadas digitalmente) caem e colidem com pinos (obstáculos), desviando-se aleatoriamente para a esquerda ou para a direita. Cada colisão é um evento independente, tornando a trajetória de uma esfera individual imprevisível.
* **Distribuição Normal:** Apesar do comportamento aleatório de cada esfera individual, a distribuição agregada das esferas nos compartimentos na base do tabuleiro tende a formar uma curva em forma de sino, característica da distribuição normal.
* **Probabilidade:** A Tábua de Galton é uma ferramenta clássica para demonstrar como uma série de eventos binomiais (desvio para esquerda ou direita com certa probabilidade em cada pino) pode levar a uma distribuição de probabilidade conhecida.

## Hardware Necessário

* Raspberry Pi Pico
* Display OLED SSD1306 (configurado para I2C, 128x64 pixels)
    * Endereço I2C: `0x3C`
    * Porta I2C: `i2c1`
    * Pino SDA: `GP14`
    * Pino SCL: `GP15`
* Joystick Analógico
    * Pino Eixo X: `GP26` (ADC0)
    * Pino Eixo Y: `GP27` (ADC1)

## Software e Bibliotecas

* **Linguagem:** C
* **SDK:** Raspberry Pi Pico SDK
* **Módulos do Projeto:**
    * `main.c`: Lógica principal, inicialização, loop de controle, interface com o usuário.
    * `ball.c` / `ball.h`: Gerenciamento do estado, movimento e desenho das esferas. Define o comportamento de queda e colisão.
    * `histogram.c` / `histogram.h`: Coleta dados sobre as posições finais das esferas e desenha o histograma resultante no display.
    * `joystick.c` / `joystick.h`: Leitura e processamento dos dados do joystick analógico para controlar a velocidade da simulação.
    * `ssd1306.c` / `ssd1306.h`: Driver para o display OLED SSD1306.
    * `font.h`: Contém os dados da fonte utilizada para desenhar texto no display.
* **Bibliotecas do Pico SDK:**
    * `pico/stdlib.h`
    * `pico/rand.h` (para a geração de números aleatórios que escolhem o desvio das esferas)
    * `hardware/i2c.h` (para comm com o display OLED)
    * `hardware/adc.h` (para leitura do joystick analógico)

## Estrutura do Projeto

O código está organizado nos seguintes arquivos principais:

* **`main.c`**:
    * Inicializa o hardware (Pico, display, joystick, buzzer).
    * Desenha os pinos no display.
    * Configura e gerencia um timer com repetição (`simulation_tick_callback`) que atualiza o estado da simulação.
    * Lê a entrada do joystick para ajustar a velocidade da simulação.
    * Controla a taxa de FPS.
* **`ball.c`**:
    * `ball_t`: Estrutura que armazena a posição (x, y) e o estado (`force_fall`) de cada esfera.
    * `MAX_BALL_QUANT`: Define o número máximo de esferas que podem estar em movimento simultaneamente.
    * `get_random_direction()`: Determina se uma esfera desvia para a esquerda ou direita (50% de chance para cada lado por padrão).
    * `draw_ball()`, `erase_ball()`: Funções para desenhar e apagar esferas no display.
    * `updt_balls_pos()`: Atualiza a posição de todas as esferas. Simula a queda e o desvio. Detecta quando uma esfera atinge a base e retorna sua posição de saída. Controla a introdução de novas esferas no topo.
* **`histogram.c`**:
    * `MAX_BINS`: Número máximo de compartimentos no histograma.
    * `init_hist()`: Inicializa os contadores dos compartimentos do histograma.
    * `addto_hist()`: Incrementa o contador do compartimento correspondente à posição onde uma esfera terminou sua queda.
    * `draw_hist()`: Desenha o histograma na lateral do display, mostrando a distribuição das esferas.
* **`joystick.c`**:
    * `init_joystick()`: Inicializa os pinos ADC para leitura do joystick.
    * `getXY_joystick()`: Lê os valores brutos do ADC e os converte para coordenadas X e Y normalizadas (-1 a 1).
    * `getRA_joystick()`: Converte as coordenadas X,Y para coordenadas polares (Raio e Ângulo). O ângulo é usado para detectar o movimento circular do joystick.
* **`ssd1306.c`**:
    * Driver completo para o display OLED SSD1306, fornecendo funções para inicialização, limpeza, desenho de pixels, linhas, quadrados, texto e bitmaps.
* **`font.h`**:
    * Define os dados para uma fonte bitmap (8x5 pixels por caractere) usada para exibir texto.

## Funcionamento da Simulação

1.  **Inicialização:** O programa inicializa o microcontrolador, o display OLED, o joystick e o buzzer. O tabuleiro da Tábua de Galton, com seus pinos, é desenhado no lado esquerdo do display. O histograma é inicializado no lado direito.
2.  **Liberação das Esferas:** As esferas são liberadas do topo central do tabuleiro, uma de cada vez, com um pequeno intervalo para evitar sobreposição inicial.
3.  **Movimento e Colisões:**
    * Em cada passo da simulação, a posição de cada esfera é atualizada.
    * As esferas movem-se para baixo (`x += 1`).
    * Quando o componente `x` da posição de uma esfera é par (simulando um nível de pinos), a esfera tem uma chance de 50% de se mover para a esquerda (`y -= 1`) e 50% de se mover para a direita (`y += 1`). O estado `force_fall` garante que o movimento horizontal ocorra apenas nesses "níveis de pinos", e o movimento vertical ocorra no passo seguinte.
4.  **Acumulação:** Quando uma esfera atinge a base do tabuleiro (sai da área dos pinos), sua posição final `y` é registrada.
5.  **Histograma:** A contagem para o compartimento (`bin`) correspondente à posição final `y` da esfera é incrementada no histograma. O histograma é redesenhado, mostrando barras cuja altura representa o número de esferas acumuladas em cada compartimento.
6.  **Controle de Velocidade:**
    * O usuário pode controlar a velocidade da simulação usando o joystick.
    * Ao mover o joystick em um movimento circular no sentido horário, a simulação acelera.
    * Ao mover no sentido anti-horário, a simulação desacelera.
    * A velocidade é ajustada alterando o `simulation_delay_tick_us` (intervalo entre os "ticks" da simulação) e o `n_updates` (número de atualizações de física por "tick" do display).
    * Um breve "bip" soa no buzzer quando a velocidade é alterada.
7.  **Display:** O display é atualizado a uma taxa máxima definida por `MAX_FPS`. A cada atualização do display, as esferas são apagadas de suas posições antigas e redesenhadas em suas novas posições, e o histograma é atualizado.

## Controles

* **Joystick:**
    * **Mover em círculo no sentido horário:** Aumenta a velocidade da simulação.
    * **Mover em círculo no sentido anti-horário:** Diminui a velocidade da simulação.
    * O joystick precisa ser movido além de um certo limite radial (`JOYSTICK_EXTEND_LIM`) para que o controle de velocidade seja ativado.

## Como Compilar e Executar

1.  **Configure o Ambiente de Desenvolvimento:**
    * Certifique-se de ter o Raspberry Pi Pico SDK configurado corretamente em seu sistema.

3.  **Compile e carregue o Projeto:**
    Usando a ferramenta de build ninja, compile e carregue o projeto na Rasp Pi Pico.

## Licença
O driver `ssd1306.c` e `ssd1306.h` são fornecidos sob a licença MIT. Para o restante do código do projeto, por favor, adicione uma licença de sua escolha se desejar distribuí-lo.

## 📜 Licença
MIT License - MIT GPL-3.0.

