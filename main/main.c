#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "freertos/FREERTOS.h"
#include "freertos/task.h"

#define TURBIDITY_SENSOR_ANALOG_PIN 1
#define TURBIDITY_SENSOR_DIGITAL_PIN 14
static const char *TAG = "turbidity sensor";

typedef struct
{
    uint32_t analog_pin;
    uint32_t digital_pin;
    bool is_init;
    bool is_error;
} turbidity_sensor_config_t;

#define TURBIDITY_SENSOR_DEFAULT_CONFIG(analog_gpio, digital_gpio) \
    {                                                              \
        .analog_pin = analog_gpio,                                 \
        .digital_pin = digital_gpio,                               \
        .is_init = false,                                          \
        .is_error = false,                                         \
    }

esp_err_t sensor_init(turbidity_sensor_config_t *turbidity_sensor_config)
{
    esp_err_t ret = ESP_OK;

    turbidity_sensor_config->is_init = true;
    gpio_config_t sensor_io_config;
    sensor_io_config.pin_bit_mask = (1ULL << turbidity_sensor_config->analog_pin);
    sensor_io_config.mode = GPIO_MODE_INPUT;
    sensor_io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    sensor_io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    sensor_io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    if (ESP_OK != gpio_config(&sensor_io_config))
    {
        ESP_LOGE(TAG, "%s(). ABORT. error configuring the analog pin", __func__);
        turbidity_sensor_config->is_init = false;
        goto err;
    }

    sensor_io_config.pin_bit_mask = (1ULL << turbidity_sensor_config->digital_pin);
    sensor_io_config.mode = GPIO_MODE_INPUT;
    sensor_io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    sensor_io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    sensor_io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    if (ESP_OK != gpio_config(&sensor_io_config))
    {
        ESP_LOGE(TAG, "%s(). ABORT. error configuring the digital pin", __func__);
        turbidity_sensor_config->is_init = false;
        goto err;
    }

err:
    return ret;
}

void app_main(void)
{
    turbidity_sensor_config_t turbidity_sensor_config = TURBIDITY_SENSOR_DEFAULT_CONFIG(TURBIDITY_SENSOR_ANALOG_PIN, TURBIDITY_SENSOR_DIGITAL_PIN);
    turbidity_sensor_config.is_init = false;
    if (ESP_OK == sensor_init(&turbidity_sensor_config))
    {
        ESP_LOGI(TAG, "sensor initialized");
    }
    // Initialize ADC1
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
    // adc1_config_channel_atten(turbidity_sensor_config.analog_pin, ADC_ATTEN_DB_0);

        while (1)
    {
        float adc_value = adc1_get_raw(ADC1_CHANNEL_0);
        float adc_value = adc1_get_raw(turbidity_sensor_config.analog_pin);
        int digital = gpio_get_level(turbidity_sensor_config.digital_pin);
        ESP_LOGI(TAG, "Raw value = %f\n", adc_value);
        // Convert ADC value to sensor value
        float sensor_value = (float)(adc_value / 4095.0) * 3.3;

        ESP_LOGI(TAG, "ADC value = %f\n", sensor_value);
        ESP_LOGI(TAG, "Digital value = %d\n", digital);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
