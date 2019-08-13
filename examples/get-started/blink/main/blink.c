/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/queue.h"

// criando uma queue global para envio de dados
QueueHandle_t sensor_queue;
QueueHandle_t atuador_queue;
QueueHandle_t logger_queue;

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 2

void sensor(void *arg)
{
        char leitura = 70;

        while (1)
        {
                printf("Tarefa de sensor\n");

                xQueueSend(sensor_queue, (void *)&leitura, 2000 / portTICK_PERIOD_MS);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

void controle(void *arg)
{

        char leitura_sensor;
	char leitura_atuador = 0;
	short leitura_sensor_logger;

        while (1)
        {
                printf("Tarefa de controle\n");

                xQueueReceive(sensor_queue, &leitura_sensor, 20000 / portTICK_PERIOD_MS);

                printf("Leitura sensor: %d \n", leitura_sensor);

                vTaskDelay(1000 / portTICK_PERIOD_MS);

		if (leitura_sensor > 50) {

			leitura_atuador = 1;
		}

		xQueueSend(atuador_queue, (void *)&leitura_atuador, 2000 / portTICK_PERIOD_MS);

                vTaskDelay(1000 / portTICK_PERIOD_MS);

		leitura_sensor_logger = leitura_sensor << 8;
                xQueueSend(logger_queue, (void *)&leitura_sensor_logger, 2000 / portTICK_PERIOD_MS);

                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

void atuador(void *arg)
{
	char leitura_controle;
	short leitura_atuador_logger;

        while (1)
        {
                printf("Tarefa de atuador\n");

		xQueueReceive(atuador_queue, &leitura_controle, 20000 / portTICK_PERIOD_MS);

                printf("Leitura atuador: %d \n", leitura_controle);

                vTaskDelay(1000 / portTICK_PERIOD_MS);

                leitura_atuador_logger = leitura_controle;
                xQueueSend(logger_queue, (void *)&leitura_controle, 2000 / portTICK_PERIOD_MS);

                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

void logger(void *arg)
{

	short leituras;
	char leitura_sensor;
	char leitura_atuador;

        while (1)
        {
                printf("Tarefa de logger\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);


                xQueueReceive(logger_queue, &leituras, 20000 / portTICK_PERIOD_MS);

		leitura_sensor = leituras >> 8;
		leitura_atuador = leituras;

                printf("Leitura sensor: %d e leitura atuador: %d \n", leitura_sensor, leitura_atuador);

        }
}

void app_main()
{

        sensor_queue = xQueueCreate(5, sizeof(char));
	atuador_queue = xQueueCreate(5, sizeof(char));
	logger_queue = xQueueCreate(5, sizeof(short));

        /* Cria a tarefa sensor */
        xTaskCreate(sensor, "sensor", 2048, NULL, 0, NULL);

        /* Cria a tarefa controle */
        xTaskCreate(controle, "controle", 2048, NULL, 0, NULL);

        /* Cria a tarefa atuador */
        xTaskCreate(atuador, "atuador", 2048, NULL, 0, NULL);

        /* Cria a tarefa logger */
        xTaskCreate(logger, "logger", 2048, NULL, 0, NULL);

        while (1)
        {

                printf("\n");
		printf("\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
