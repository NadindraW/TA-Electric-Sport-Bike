/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 29/04/23
 *
 * VOTOL Communication : 
 * - SEIN = UNCONFIRMED
 * - HIGHBEAM, MODE, STATE, SPEED, BATTERY = OKAY
 * - THORTTLE = UNCONFIRMED
 * 
 * NEXTION Communication = OKAY
 * IMU for GYRO SEIN     = ON PROGRESS
 * SPEAKER for Media     = OKAY
 * EEPROM for Odometer   = OKAY
 * MICROSD for Logging   = ON PROGRESS
 *
 * INTEGRATION in FREERTOS = UNCONFIRMED
 */
 
#include "FS.h"
#include "SD.h"
#include "SPI.h"
// #include "BluetoothSerial.h"  // This library allows us to communicate using EPS32 Serial BT.

/* Define the GPIO that is used for electrice vehicle hardware.
 * Make sure that ESP32 has compatible supporting board to handle the voltage of hardware.
 */

// V1 : DEFAULT (IMU, I2S)
// V2 : INCLUDE MICROSD 
// V3 : INCLUDE OCTOCOUPLER/RELAY

#define RXD2 16
#define TXD2 17
#define THROT_PIN 2     // V1: 2  V2: 2  V3:34 
#define BEAMS_PIN 4     // V1:27  V2: 4  V3:35
#define SEINR_PIN 32    // V1:32  V2:32  V3:32
#define SEINL_PIN 33    // V1:33  V2:33  V3:33 
#define I2S_LRC_PIN 25  // V1:25  V2:25  V3:25 
#define I2S_BLK_PIN 26  // V1:26  V2:26  V3:26 
#define I2S_GAN_PIN 27  // V1:23  V2:27  V3:27 
#define IMU_SDA_PIN 21  // V1:21  V2:21  V3:21 
#define IMU_SCL_PIN 22  // V1:22  V2:22  V3:22
#define SDC_CS_PIN   5  // V1: -  V2: 5  V3: 5
#define SDC_CLK_PIN  18 // V1: -  V2:18  V3:18 
#define SDC_MISO_PIN 19 // V1: -  V2:19  V3:19 
#define SDC_MOSI_PIN 23 // V1: -  V2:23  V3:23 
// #define DSEINR_PIN   0  // V1: -  V2: -  V3: 0
// #define DSEINL_PIN   2  // V1: -  V2: -  V3: 2

String IMU_record;
double angleC1=0, angleC2=0;
int16_t aX, aY, aZ, gX, gY, gZ, temperature;
const char *txtname;

bool mediaValue;
uint8_t inDataVotol[30];  // (Feedback) Data received by VOTOL 
uint8_t val_sein;
uint16_t mVotolVoltage;
uint16_t mVotolCurrent;
uint16_t mVotolRPM;
uint16_t mVotolConTemp;
uint16_t mVotolExtTemp;
uint16_t mVotolMode  = 3; 
uint16_t mVotolState = 3;
// uint32_t mVotolFaultCode;

/* Classic Bluetooth Configuration. -----------------------------------------------------------
 * If activated, then smartphone is able to retrive data from ESP32. 
 */
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif
// #if !defined(CONFIG_BT_SPP_ENABLED)
// #error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
// #endif
// String device_name = "EV-PENSVKTR BT General";
// BluetoothSerial SerialBT;

/* Real-Time Operating System (RTOS) Configuration. -------------------------------------------
 * These config are needed for handle multiple task with various priorities.
 */
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// Define task that it will be running on parallel scheduling
void TaskImu(void *pvParamenters);
void TaskCommunication(void *pvParameters);
// void TaskMedia(void *pvParameters);

// Define task_handle for suspend, resume, delete task when program is running.
// TaskHandle_t media_task_handle;

/* -------------------------------- TASK INITIALIZATION ----------------------------------------
 * ---------------------------------------------------------------------------------------------
 */
void setup() {
    Serial.begin(9600);                             // Serial to Nextion Display
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  // Serial to Votol Data Protocol
    // SerialBT.begin(device_name);                 // Serial Bluetooth General
    // mediaBluetoothSetup();                        // Serial Bluetooth Speaker

    // Data Logger Setup
    if(!SD.begin(5)) Serial.println("Card Mount Failed");
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE) Serial.println("No SD card attached");

    // Set Up two tasks to run independently
    uint32_t sampling_delay = 1000;                           // Sampling delay for odometer  
    xTaskCreatePinnedToCore(TaskImu, "Task Imu", 2048, NULL, 2, NULL,  ARDUINO_RUNNING_CORE);
    xTaskCreate(TaskCommunication, "Task Comms", 2048, NULL, 1, NULL);
    // xTaskCreatePinnedToCore(TaskMedia, "Task Media", 2048, NULL, 1, NULL, 1);

    Serial.printf("Basic Multi Threading on FreeRTOS : Running...\n");

    deleteFile(SD, "/log_old.txt");
    renameFile(SD, "/log.txt", "/log_old.txt");
    writeFile(SD, "/log.txt", IMU_record.c_str());
}

void loop() {
    /* Because the existing regulation, audio player should not be played when
     * electric vehicle in drive mode. Thus, this program to handle audio player feature.
     * Audio player only activated if vehicle in parking mode.
     */

    // if(val_state != 3 && mediaValue == true){
    //   delay(1000); 
    //   // vTaskSuspend(media_task_handle);
    //   mediaBluetoothEnd();
    // } else if (val_state == 3 && mediaValue == false){
    //   delay(1000); 
    //   // vTaskResume(media_task_handle);
    //   mediaBluetoothStart();
    // }

    IMU_record = "{\"Roll\":" + String(angleC2) + ",\"Pitch\":" + String(angleC1) + ",\"Yaw\":" + String(gX) +"}\n";
    appendFile(SD, "/log.txt", IMU_record.c_str());
    delay(50);
}

/* ---------------------------------------------- TASK LIST ---------------------------------------- 
 * -------------------------------------------------------------------------------------------------
 */

void TaskCommunication(void *pvParameters){
    pinMode(SEINR_PIN, INPUT);
    pinMode(SEINL_PIN, INPUT);
    pinMode(BEAMS_PIN, INPUT);
    for (;;){
        uart_send_dashboard_UI();
    }
}

void TaskImu(void *pvParameters){
    i2c_imu_setup();
    for (;;){
        i2c_get_rawimu();
        complementaryFilter();
        delay(50);
    }  
}

void TaskMedia(void *pvParameters){
    mediaBluetoothSetup();
    for (;;){
        mediaSetVolume();
        delay(100);
    }
}
