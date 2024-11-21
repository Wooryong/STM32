/*
 * LCD1602.c
 *
 *  Created on: Nov 21, 2024
 *      Author: user
 *
 * Function : 1602 LCD Device Control
 */

extern I2C_HandleTypeDef *hi2c;
// extern I2C_HandleTypeDef hi2c1;
#define I2C_ADDR 0X4E		// (0X27 << 1)

void LCD_Command(char CMD) // CMD_Bit(8-bit) : ABCD_EFGH
{
	char n1, n2; // Nibble(4-bit)
	char dd[4]; //
	n1 = CMD & 0XF0; // Upper Nibble > n1 = ABCD_0000
	n2 = (CMD << 4); // Lower Nibble > n2 = EFGH_0000

	dd[0] = n1 | 0X0C; // n1 Disable
	dd[1] = n1 | 0X08; // n1 Enable
	dd[2] = n2 | 0X0C; // n2 Disable
	dd[3] = n2 | 0X08; // n2 Enable

	HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, dd, 4, 10);
	// HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

}

void LCD_Data(char ch)
{


}
void LCD_init() // LCD Initialize
{
	LCD_Command(0X01); // Screen Clear
	HAL_Delay(1); //
	LCD_Command(0X02); // Cursor Position : Home
	HAL_Delay(1); //
	LCD_Command(0X06); //
	HAL_Delay(1); //
	LCD_Command(0X0F); //
	HAL_Delay(1); //
}
