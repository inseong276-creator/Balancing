#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void Serial_USART2_Init(void);
void Serial_WriteChar(char c);
void Serial_WriteString(const char *s);
void Serial_WriteInt(int32_t value);
void Serial_WriteFloat2(float value);
void Serial_USART3_Init(void);
void Read_CMD(void);

#endif