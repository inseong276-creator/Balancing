#ifndef SENSOR_H
#define SENSOR_H

#include "stm32f103xb.h"
#include <stdint.h>

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_RawData;

void Sensor_I2C2_Init(void);
int Sensor_MPU6050_Init(void);
int Sensor_MPU6050_ReadRaw(MPU6050_RawData *raw);

float Sensor_GetPitchAccDeg(int16_t ax, int16_t az);
float Sensor_GetGyroYDegPerSec(int16_t gy_raw);

void Sensor_FilterReset(float pitch_init);
float Sensor_ComplementaryFilter(float pitch_prev, float gyro_rate, float pitch_acc, float dt);

void Sensor_PID_Reset(void);
float Sensor_PID_Control(float target_angle, float pitch, float pitch_rate, float dt);

void Print_SensorLog(float pitch_acc, float pitch, float control);


#endif