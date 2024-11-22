/*
 * LCD1602.c
 *
 *  Created on: Nov 21, 2024
 *      Author: user
 *
 * Function : 1602 LCD Device Control
 */

#include "main.h"
// #include "C:\Users\user\STM32Cube\Repository\STM32Cube_FW_F4_V1.28.1\Drivers\STM32F4xx_HAL_Driver\Inc\stm32f4xx_hal_i2c.h"
// for later use of "I2C_HandleTypeDef"

// extern I2C_HandleTypeDef hi2c1;
// I2C_HandleTypeDef *hi2c;
extern I2C_HandleTypeDef *hi2c;
#define I2C_ADDR 0X4E		// Slave Address for I2C // (0X27 << 1)

I2C_HandleTypeDef *hi2c = NULL;
// I2C is not default device > can't use "extern" in general-purpose
// If I2C is used, *hi2c will be re-defined in "main.c".
// Otherwise, *hi2c will be remained as NULL.

int i2c_init(I2C_HandleTypeDef *p)
// void i2c_init(void *p)
{
	hi2c = p;
}

int i2c_scan() // Find Slave Address for I2C Communication
{
	// If I2C is used, this function does nothing.
	if(hi2c == NULL) return;

	printf("\n");
	printf("Check Slave Address for I2C ... \r\n");
	for (int addr = 0; addr < 128; addr++)
	{
		if (HAL_I2C_IsDeviceReady(hi2c, addr, 1, 10) == HAL_OK) printf("  %02x ", addr);
		// HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
		else printf("  .  ", addr);

		if ( ((addr + 1) % 16) == 0 ) printf("\r\n");
	}
}

void LCD_Command(char CMD) // CMD_Bit(8-bit) : ABCD_EFGH
{
	// Upper 4-bit (CMD) & Lower 4-bit (Control)
	char n1, n2, n3, n4; // Nibble(4-bit)
	char dd[4]; //

	n1 = CMD & 0XF0; // Upper Nibble > n1 = ABCD_0000
	n2 = (CMD << 4) & 0XF0; // Lower Nibble > n2 = EFGH_0000

	n3 = (1 << 3) | (1 << 2) | 0 | 0; // RW | EN = 1 | NC | RS;
	n4 = (1 << 3) | 0        | 0 | 0; // RW | EN = 0 | NC | RS;

	// Control 4-bit : {RW, Enable, NC, RS}
	// RW '1' = Write Operation & RW '0' = Read Operation
	// Enable '1' -> '0' or '0' -> '1' = Actual Transmit
	// NC : No Connect >> Always '0'
	// RS '0' = Instruction (Command) & RS '1' = Data
	// Control 4-bit {1 1 0 0} > {1 0 0 0} = Write Upper 4-bit Command
	// Control 4-bit {1 1 0 1} > {1 0 0 1} = Write Upper 4-bit Data
	dd[0] = n1 | n3; // n1 Enable >> 8'b ABCD_1100
	dd[1] = n1 | n4; // n1 Disable >> 8'b ABCD_1000
	dd[2] = n2 | n3; // n2 Enable >> 8'b EFGH_1100
	dd[3] = n2 | n4; // n2 Disable >> 8'b EFGH_1000

	HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, dd, 4, 10);
	// HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

}

void LCD_Data(char Data) // Data_Bit(8-bit) : ABCD_EFGH
{
	// Upper 4-bit (Data) & Lower 4-bit (Control)
	char n1, n2, n3, n4; // Nibble(4-bit)
	char dd[4]; //

	n1 = Data & 0XF0; // Upper Nibble > n1 = ABCD_0000
	n2 = (Data << 4) & 0XF0; // Lower Nibble > n2 = EFGH_0000

	n3 = (1 << 3) | (1 << 2) | 0 | (1 << 0); // RW | EN = 1 | NC | RS = 1;
	n4 = (1 << 3) | 0        | 0 | (1 << 0); // RW | EN = 0 | NC | RS = 1;

	dd[0] = n1 | n3; // n1 Enable >> 8'b ABCD_1100
	dd[1] = n1 | n4; // n1 Disable >> 8'b ABCD_1000
	dd[2] = n2 | n3; // n2 Enable >> 8'b EFGH_1100
	dd[3] = n2 | n4; // n2 Disable >> 8'b EFGH_1000

	HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, dd, 4, 10);
	// HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
}
void LCD_init() // LCD Initialize Sequence
{
	/*
	char CMD_Array[] = { 0X0F, 0X06, 0X02, 0X01 };

	for (int i = 0; i < sizeof(CMD_Array); i++)
	{
		LCD_Command( CMD_Array[i] );
		HAL_Delay(10);
	}
	*/

	LCD_Command(0X01); // Screen Clear
	HAL_Delay(10); //
	LCD_Command(0X02); // Cursor Position : Home
	HAL_Delay(10); //
	LCD_Command(0X06); // Entry Mode Set (Increment Mode & No Shift)
	HAL_Delay(10); //
	LCD_Command(0X0F); // Display Switch (Display ON, Cursor ON, Blink ON)
	// 0000_1 D C B
	HAL_Delay(10); //
}

void LCD_Print(char *str) // Print Character String
{
	while (*str) // If Null, break;
	{
		LCD_Data(*str);
		str++; // Next Character
	}
}

void LCD_PrintEx(char *str, int Line)
{
	Line ? LCD_Command(0XC0) : LCD_Command(0X80);
	/*
	if (Line == 0)	LCD_Command(0X80); // Line 1
	else				LCD_Command(0XC0); // Line 2
	*/
	LCD_Print(str);
}
