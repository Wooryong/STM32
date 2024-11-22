/*
 * myLib.c
 *
 *  Created on: Nov 15, 2024
 *      Author: user
 */

#include "main.h"

extern UART_HandleTypeDef huart2; // extern


int __io_putchar(int ch) // Single Character Output to PC
{
	HAL_UART_Transmit(&huart2, &ch, 1, 10);
	// HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
	// *pData : Buffer for Transmitted Data
	// uint16_t Size : Size of Transmitted Data >> 1 (STM32 : Little Endian > int 4-byte : [ch][\0][\0][\0]
	// integer : 4-byte vs. char : 1-byte
	// Timeout : Response Time < 10 ms
	// Baud rate = 9600 = 1k bps >
	return ch;
}

void CLS() // Screen Clear
{
	printf("\033[2J");
	// [Escape Sequence] ESC[2J ; ESC (27) : Octal 033
}

void Cursor(int x, int y) // Move Cursor
{
	char buf[20];
	sprintf(buf, "\033[%d;%dH", y, x); // sprintf(buf , "xxxx %d xxx" , a)
	puts(buf); // puts(str) : Print character String
	// printf("%s", buf);
	// [Escape Sequence] - ESC[0;0H // ESC[(y좌표);(x좌표)H
}

void Standby()
{
	while ( HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) );
	// BTN B1 : OFF = 'H', ON = 'L'
}

void ProgramStart(char* str)
{
	CLS();
	Cursor(0, 0); // Cursor Position X=0, Y=0

	// Print-out Startup Instruction
	printf("Program Name - %s \r\n", str);
	printf("Press Blue-Button (B1) to Start ... \r\n");

	Standby();
}

