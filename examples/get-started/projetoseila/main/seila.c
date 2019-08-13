#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID "iPhone de Vinicio"


/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event 
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "iPhone de Vinicio",
            .password = "12345678"
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}
// end wifi config

// criando uma queue global para envio de dados
QueueHandle_t queue_sensor;
QueueHandle_t queue_controle;
QueueHandle_t queue_atuador;
QueueHandle_t queue_logger;
QueueHandle_t queue_mqtt;

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 2

void sensor(void *arg)
{
        char insere_queue_sensor;

        while (1)
        {

		insere_queue_sensor = (rand() %100);

                printf("Tarefa de sensor\n");

                xQueueSend(queue_sensor, (void *)&insere_queue_sensor, 2000 / portTICK_PERIOD_MS);
                vTaskDelay(200 / portTICK_PERIOD_MS);
        }
}

void controle(void *arg)
{

        char leitura_queue_sensor;

        while (1)
        {
                printf("Tarefa de controle\n");

                xQueueReceive(queue_sensor, &leitura_queue_sensor, 20000 / portTICK_PERIOD_MS);
		printf("sensor: %d\n", leitura_queue_sensor);

                vTaskDelay(100 / portTICK_PERIOD_MS);
        }
}

void atuador(void *arg)
{

        while (1)
        {
                printf("Tarefa de atuador\n");

                vTaskDelay(100 / portTICK_PERIOD_MS);
        }
}

void logger(void *arg)
{

        while (1)
        {
                printf("Tarefa de logger\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);

        }
}

void mqtt(void *arg)
{

        while (1)
        {
                printf("Tarefa de mqtt\n");
                vTaskDelay(5000 / portTICK_PERIOD_MS);

        }
}

void app_main()
{

	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();

        queue_sensor = xQueueCreate(5, sizeof(char)); //8 bits ou 1 byte e short 16 bits ou 2 bytes
	queue_controle = xQueueCreate(5, sizeof(char));
	queue_atuador = xQueueCreate(5, sizeof(char));
	queue_logger = xQueueCreate(12, sizeof(char));
	queue_mqtt = xQueueCreate(5, sizeof(char));

        /* Cria a tarefa sensor */
        xTaskCreate(sensor, "sensor", 2048, NULL, 5, NULL);

        /* Cria a tarefa controle */
        xTaskCreate(controle, "controle", 2048, NULL, 5, NULL);

        /* Cria a tarefa atuador */
        xTaskCreate(atuador, "atuador", 2048, NULL, 5, NULL);

        /* Cria a tarefa logger */
        xTaskCreate(logger, "logger", 2048, NULL, 2, NULL);

        /* Cria a tarefa mqtt */
        xTaskCreate(mqtt, "mqtt", 2048, NULL, 1, NULL);

        while (1)
        {

                printf("\n");
		printf("\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
