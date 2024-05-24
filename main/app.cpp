#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_timer.h"  // Para medir o tempo em microsegundos
#include "serial.h"
#include "digital.h"

// Pinos do sensor ultrassônico: GPIO10(s3) como echo e GPIO09(s2) como trigger

extern "C" void app_main();

void ultrasonic_task(void *pvParameters);
uint32_t calibrate_height(void);

uint32_t calibration_height = 0;

void app_main() {
    digital.pinMode(PIN10, OUTPUT);
    digital.pinMode(PIN9, INPUT);

      serial.begin(9600);
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
        digital.digitalWrite(PIN9, LOW);
        ets_delay_us(2);
        digital.digitalWrite(PIN9, HIGH);
        ets_delay_us(10);
        digital.digitalWrite(PIN9, LOW);

        // Emite um sinal e conta o tempo até esse sinal retornar
        while (digital.digitalRead(PIN10) == 0) {
            start = esp_timer_get_time();
        }
        while (digital.digitalRead(PIN10) == 1) {
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

    digital.digitalWrite(PIN9, LOW);
    ets_delay_us(2);
    digital.digitalWrite(PIN9, HIGH);
    ets_delay_us(10);
    digital.digitalWrite(PIN9, LOW);

    // Emite um sinal e conta o tempo até esse sinal retornar
    while (digital.digitalRead(PIN10) == 0) {
        start = esp_timer_get_time();
    }
    while (digital.digitalRead(PIN10) == 1) {
        end = esp_timer_get_time();
    }

    // Usa os valores de tempo para calcular a distância
    distance = (end - start) * 0.034 / 2;
    printf("Distância calibrada: %u cm\n", distance);

    return distance;
}