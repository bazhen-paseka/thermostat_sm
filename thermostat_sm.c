/**
* \file
* \version 1.0
* \author bazhen.levkovets
** \date 2018
*
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*************************************************************************************
*
* \brief
*
*/

/*
**************************************************************************
*							INCLUDE FILES
**************************************************************************
*/
	#include <thermostat_sm.h>
/*
**************************************************************************
*							LOCAL DEFINES
**************************************************************************
*/

/*
**************************************************************************
*							LOCAL CONSTANTS
**************************************************************************
*/
/*
**************************************************************************
*						    LOCAL DATA TYPES
**************************************************************************
*/
/*
**************************************************************************
*							  LOCAL TABLES
**************************************************************************
*/
/*
**************************************************************************
*								 MACRO'S
**************************************************************************
*/

/*
**************************************************************************
*						    GLOBAL VARIABLES
**************************************************************************
*/

		lcd1602_fc113_struct h1_lcd1602_fc113 =
		{
			.i2c = &hi2c1,
			.device_i2c_address = ADR_I2C_FC113
		};

		int cnt_int;

		char ds18b20_rom_1[8] = {0x28, 0xFF, 0x1F, 0x4C, 0x23, 0x17, 0x03, 0xB9};
		char ds18b20_rom_2[8] = {0x28, 0xFF, 0xB0, 0x4E, 0x23, 0x17, 0x03, 0xE2};

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/
	
/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void Thermostat_Init(void) {

	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\tThermoStat 2020-march-27 v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	LCD1602_Init(&h1_lcd1602_fc113);
	I2C_ScanBus_to_LCD1602(&h1_lcd1602_fc113);
	LCD1602_Clear(&h1_lcd1602_fc113);
	HAL_GPIO_TogglePin(RELAY_2_GPIO_Port, RELAY_2_Pin);
}
//************************************************************************

void Thermostat_Main(void) {
	  HAL_GPIO_TogglePin(LED_BOARD_GPIO_Port,LED_BOARD_Pin);
	  HAL_GPIO_TogglePin(RELAY_1_GPIO_Port, RELAY_1_Pin);


	  //DS18b20_ConvertTemp_SkipROM();
	  DS18b20_ConvertTemp_MatchROM(ds18b20_rom_1);
	  DS18b20_ConvertTemp_MatchROM(ds18b20_rom_2);

	  HAL_Delay(1000);
	  HAL_GPIO_TogglePin(RELAY_2_GPIO_Port, RELAY_2_Pin);

	  //int temp = DS18b20_Get_Temp_SkipROM()/16;
	  int temp1 = DS18b20_Get_temp_MatchROM(ds18b20_rom_1)/16;
	  int temp2 = DS18b20_Get_temp_MatchROM(ds18b20_rom_2)/16;

		char DataChar[100];
		sprintf(DataChar,"%d) %d; %d;\r\n", cnt_int++, temp1, temp2);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));
		LCD1602_Cursor_Return(&h1_lcd1602_fc113);
		//DS18b20_Print_serial_number(&huart1);
		HAL_Delay(4000);
}
//-------------------------------------------------------------------------------------------------


//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

//************************************************************************
