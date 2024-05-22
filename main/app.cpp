#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"  // Para medir o tempo em microsegundos
#include "serial.h"

// Pinos do sensor ultrassônico
#define TRIGGER_GPIO 5
#define ECHO_GPIO 4

#define BUF_SIZE 128

void ultrasonic_task(void *pvParameters);
uint32_t calibrate_height(void);

uint32_t calibration_height = 0;

void app_main() {
    gpio_set_direction(TRIGGER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);

    char opcao = serial.readChar();
    
    printf("Para realizar a calibragem você precisa instalar o sensor na posição final dele. Após, entrar com a opção 1 no menu.\n");
    while(true){
        if(opcao == '1') {
            calibrate_height();
        }
    }

    xTaskCreate(ultrasonic_task, "medicao_altura", 2048, NULL, 1, NULL);
}

void ultrasonic_task(void *pvParameters) {
    printf("Medindo altura\n");
    uint32_t measured_height = 0;
    uint32_t distance = 0;
    uint64_t start = 0;
    uint64_t end = 0;
    char buffer[20];

    while (1) {
        gpio_set_level(TRIGGER_GPIO, 0);
        ets_delay_us(2);
        gpio_set_level(TRIGGER_GPIO, 1);
        ets_delay_us(10);
        gpio_set_level(TRIGGER_GPIO, 0);

        // Emite um sinal e conta o tempo até esse sinal retornar
        while (gpio_get_level(ECHO_GPIO) == 0) {
            start = esp_timer_get_time();
        }
        while (gpio_get_level(ECHO_GPIO) == 1) {
            end = esp_timer_get_time();
        }

        // Usa os valores de tempo para calcular a distância
        distance = (end - start) * 0.034 / 2;

        if (calibration_height == 0) {
            printf("Precisa de calibração! Pressione 1 para calibrar.\n");
        } else {
            // Calcula a altura da pessoa
            measured_height = calibration_height - distance;
            sprintf(buffer, "Altura: %d cm\n", measured_height);
            printf("%s", buffer);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

uint32_t calibrate_height(void) {
    uint32_t distance = 0;
    uint64_t start = 0;
    uint64_t end = 0;

    printf("Calibrando...\n");

    gpio_set_level(TRIGGER_GPIO, 0);
    ets_delay_us(2);
    gpio_set_level(TRIGGER_GPIO, 1);
    ets_delay_us(10);
    gpio_set_level(TRIGGER_GPIO, 0);

    // Emite um sinal e conta o tempo até esse sinal retornar
    while (gpio_get_level(ECHO_GPIO) == 0) {
        start = esp_timer_get_time();
    }
    while (gpio_get_level(ECHO_GPIO) == 1) {
        end = esp_timer_get_time();
    }

    // Usa os valores de tempo para calcular a distância
    distance = (end - start) * 0.034 / 2;
    printf("Distância calibrada: %u cm\n", distance);

    return distance;
}