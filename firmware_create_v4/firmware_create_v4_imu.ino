/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 29/04/23
 */

#include "Wire.h"                // This library allows us to communicate with I2C devices.

/* Default I2C address of the MPU-6050 is 0x68. ---------------------------------------------
 * If AD0 pin is set to HIGH, the I2C address will be 0x69. These variable
 * provide raw data of both accelerometer and gyro.
 * For Spesific purpose we declare angle value C1,C2,C3 and Temperature data 
 */

/* These variables are specificly for GY-521 (MPU60050) */
const int MPU_ADDR = 0x68;

char tmp_str[7];                        // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
    sprintf(tmp_str, "%6d", i);
    return tmp_str;
}

void i2c_imu_setup(){
    /* GY-521 Sensor Module Setup */
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(0x6B);                 // PWR_MGMT_1 register
    Wire.write(0);                    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
}

void i2c_get_rawimu(){
    /* Uncomment if we use GY-521 Sensor Module */
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);                      // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false);           // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
    // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
    aX = Wire.read()<<8 | Wire.read();          // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    aY = Wire.read()<<8 | Wire.read();          // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    aZ = Wire.read()<<8 | Wire.read();          // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
    gX = Wire.read()<<8 | Wire.read();          // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    gY = Wire.read()<<8 | Wire.read();          // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    gZ = Wire.read()<<8 | Wire.read();          // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

}

void complementaryFilter(){
    angleC1 = 0.9 * (angleC1 + float(gY)*0.01/131) + 0.1 * atan2((double)aX,(double)aZ)*180/PI;
    angleC2 = 0.9 * (angleC2 + float(gX)*0.01/131) + 0.1 * atan2((double)aY,(double)aZ)*180/PI;

    /* Decision Maker for GY-521*/
    if (angleC2 > 10) {
        val_sein = 2;
        // Serial.print("Kanan"); 
    } else if (angleC2 < -10) {
        val_sein = 1;
        // Serial.print("Kanan"); 
    } else {
        val_sein = 0;
        // Serial.print("Lurus"); 
    }     
}