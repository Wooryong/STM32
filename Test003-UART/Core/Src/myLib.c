/*
 * myLib.c
 *
 *  Created on: Nov 15, 2024
 *      Author: user
 */

int __io_putchar(int ch) // Single Character Output to PC
{
	HAL_UART_Transmit(&huart2, &ch, 1, 10);
	// HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
	// *pData : Buffer for Transmitted Data
	// uint16_t Size : Size of Transmitted Data >> 1
	// integer : 4-byte vs. char : 1-byte
	// Timeout : Response Time < 10 ms
	// Baud rate = 9600 = 1k bps >
	return ch;
}

void Standby()
{

	while ( HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) );
	// BTN B1 : OFF = 'H', ON = 'L'
}

void ProgramStart(char* str)
{
	// Screen Clear
	printf("\033[2J"); // (Escape Sequence) - ESC[2J ; ESC (27) : Octal 033

	// printf("\033[2J\033[0;0H");
	// Move Cursor to Home Position (Left-top)
	printf("\033[0;0H"); // (Escape Sequence) - ESC[0;0H // ESC[(y좌표);(x좌표)H

	// Print-out Startup Instruction
	printf("Program Name - %s \r\n", str);
	printf("Press Blue-Button (B1) to Start ... \r\n");

	Standby();
}

