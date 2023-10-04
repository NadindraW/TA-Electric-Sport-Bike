/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 29/04/23
 */

#include "BluetoothA2DPSink.h"   

BluetoothA2DPSink a2dp_sink;

void i2s_bt_media_setup(){
    static const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,                          // corrected by info from bluetooth
        .bits_per_sample = (i2s_bits_per_sample_t) 16, // the DAC module will only take the 8bits from MSB
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear=true
    };

    static const i2s_pin_config_t i2s_pin_config = {
        .bck_io_num   = I2S_BLK_PIN, //GPIO_NUM_26, //26,
        .ws_io_num    = I2S_LRC_PIN, //GPIO_NUM_25, //25,
        .data_out_num = I2S_GAN_PIN, //GPIO_NUM_27, //23,
        .data_in_num  = -1 //I2S_PIN_NO_CHANGE
    };

    a2dp_sink.set_pin_config(i2s_pin_config);
    a2dp_sink.set_i2s_config(i2s_config);
    i2s_bt_media_start();
}

void i2s_bt_media_start(){
    a2dp_sink.start("EV-PENSVKTR BT Speaker");
    // a2dp_sink.set_volume(100);
    val_i2s = true;
}

void i2s_bt_media_end(){
    a2dp_sink.end();
    val_i2s = false;
}

void i2s_bt_media_set_volume(){
    // int sensorValue = analogRead(15);
    // int volume = sensorValue * 255.0 / 4096.0;
    // a2dp_sink.set_volume(volume);
    // Serial.println(volume);
}
