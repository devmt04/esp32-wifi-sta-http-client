#include "MAX7219.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include <string.h>

#define NUM_MODULES 4  

#define CS_LOW()  gpio_set_level(CS_PIN, 0)
#define CS_HIGH() gpio_set_level(CS_PIN, 1)

spi_device_handle_t spi;

// init CS pin
static esp_err_t init_cs(){
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << CS_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(CS_HIGH());   // Deselect MAX7219
    return ESP_OK;
}

// Send one register+data pair to ALL cascaded modules
void max7219_send_all(uint8_t reg, uint8_t data)
{
    uint8_t buf[NUM_MODULES * 2];

    for (int i = 0; i < NUM_MODULES; i++) {
        buf[i*2 + 0] = reg;
        buf[i*2 + 1] = data;
    }

    spi_transaction_t t = {
        .length = NUM_MODULES * 16,
        .tx_buffer = buf
    };

    CS_LOW();
    spi_device_polling_transmit(spi, &t);
    CS_HIGH();
}

void max7219_send(int module, uint8_t reg, uint8_t data)
{
    uint8_t buf[NUM_MODULES * 2];

    for (int i = 0; i < NUM_MODULES; i++) {
        if (i == module) {
            buf[i*2 + 0] = reg;
            buf[i*2 + 1] = data;
        } else {
            buf[i*2 + 0] = 0x00;   // NO-OP register
            buf[i*2 + 1] = 0x00;
        }
    }

    spi_transaction_t t = {
        .length = NUM_MODULES * 16,
        .tx_buffer = buf
    };

    CS_LOW();
    spi_device_polling_transmit(spi, &t);
    CS_HIGH();
}

static void max7219_basic_init()
{
    max7219_send_all(0x0C, 0x01);  // Normal operation
    max7219_send_all(0x09, 0x00);  // Decode OFF
    max7219_send_all(0x0A, 0x0F);  // Brightness MAX
    max7219_send_all(0x0B, 0x07);  // Scan limit = 8 rows
    max7219_send_all(0x0F, 0x00);  // Test mode OFF
}

void drawChar_8x8(int module, char c){
    for (int i = 0; i < sizeof(font8x8)/sizeof(font8x8[0]); i++) {
        if (font8x8[i].c == c) {
            for (int row = 0; row < 8; row++) {
                max7219_send(module, row+1, font8x8[i].rows[row]);
            }
            return;
        }
    }
}

void drawChar_4x4(int module, char c, int quadrant){
    int offset_row = (quadrant > 1) ? 4 : 0;
    int offset_col = (quadrant % 2) ? 4 : 0;

    for (int i = 0; i < sizeof(font4x4)/sizeof(font4x4[0]); i++) {
        if (font4x4[i].c == c) {

            for (int r = 0; r < 4; r++) {

                uint8_t rowdata = (font4x4[i].rows[r] & 0x0F);

                // Shift into correct 4×4 quadrant
                rowdata <<= (4 - offset_col);

                // Read existing row (not stored — overwrite fully)
                max7219_send(module, offset_row + r + 1, rowdata);
            }
            return;
        }
    }
}

void drawClear(int module){
    for (int row = 1; row <=8; row++){
        max7219_send(module, row, 0x00);
    }
}

void drawClear_4x4(int module, int quadrant){
    int offset_row = (quadrant > 1) ? 4 : 0;
    int offset_col = (quadrant % 2) ? 4 : 0;

    for (int r = 0; r < 4; r++) {
        // Read existing row (not stored — overwrite fully)
        max7219_send(module, offset_row + r + 1, 0x00);
    }
}

void drawClear_all(){
    for (int module = 0; module < NUM_MODULES; module++){
        drawClear(module);
    }
}

void drawString_8x8(const char* str){
    char buffer[128];
    strncpy(buffer, str, sizeof(buffer));
    
    drawClear_all();
    int len = strlen(buffer);
    if(len <= 4){
        for (int i = 0; i < len; i++){
            drawChar_8x8(i, buffer[i]);
        }
    }else {
        // Animate it
    }
}

void drawString_4x4(const char* str){
    char buffer[128];
    strncpy(buffer, str, sizeof(buffer));
    
    drawClear_all();
    int len = strlen(buffer);
    if(len <= 16){
        for(int i = 0; i<len; i++){
            int module = i / 4;
            int quadrant = i % 4;
            drawChar_4x4(module, buffer[i], quadrant);
        }
    }else {
        // Animate it
    }
}

void drawPattern_8x8(int module, uint8_t pattern[8]){
    for (int row = 0; row < 8; row++) {
        max7219_send(module, row + 1, pattern[row]);
    }
}

void drawCustom(){

}

static void test_pattern(char *d)
{
    uint8_t pattern[8] = {
        0b00100010,
        0b00100010,
        0b00111110,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000010,
        0b00000000
    };

    // drawClear_all();
    // drawChar_4x4(0, 'L', 0);
    // drawChar_4x4(0, 'O', 1);
    // drawChar_4x4(1, 'O', 2);
    // drawChar_4x4(1, 'E', 1);
    // drawChar_4x4(2, 'V', 1);
    
    // drawChar_4x4(3, 'E', 3);
    // drawChar_4x4(2, 'S', 1);
    // drawChar_4x4(2, ' ', 0);
    // drawChar_4x4(3, 'L', 0);
    // drawChar_4x4(3, 'I', 1);
    // drawChar_4x4(0, 'F', 2);
    // drawChar_4x4(0, 'E', 3);
    // drawString_4x4("LOVE IS LIFE");
    drawString_8x8(d);
    // drawChar_8x8(1, 'V');

    // uint8_t pattern[8] = font8x8[0].rows;

    // for (int row = 0; row < 8; row++) {
        // max7219_send_all(row + 1, pattern[row]);
        // max7219_send_all(row + 1, font8x8['M' - 'A'].rows[row]);
        // max7219_send(2, row + 1, font8x8['M' - 'A'].rows[row]);
    // }
}


void test_draw(char *d){
    test_pattern(d);
}

esp_err_t init_spi(){
    init_cs();
    const spi_bus_config_t bus_config = {
        .miso_io_num = -1,
        .mosi_io_num = GPIO_NUM_11,
        .sclk_io_num = GPIO_NUM_12,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE,
        .data_io_default_level = 0,
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_DISABLED));

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 10 * 1000 * 1000,   // MAX7219 supports up to 10 MHz
        .mode = 0,                            // CPOL=0, CPHA=0 (SPI mode 0)
        .spics_io_num = -1,                   // MANUAL CS (mandatory for cascaded modules)
        .queue_size = 1,                      // Only 1 transaction needed
        .flags = SPI_DEVICE_HALFDUPLEX,       // MAX7219 is write-only
        .command_bits = 0,                    // MAX7219 uses simple 16-bit frames
        .address_bits = 0,                    // No address phase
        .dummy_bits = 0,                      // No dummy cycles
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_config, &spi));
    
    max7219_basic_init();
    // test_pattern(d);

    return ESP_OK;
}

