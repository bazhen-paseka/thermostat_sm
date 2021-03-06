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
	//	PA9		UART1_TX	115200
	//	PA10	UART1_RX
	//	PB6		I2C1_CLK
	//	PB7		I2C1_DATA
	//	PA6		DQ_WRITE
	//	PA7		DQ_READ
	//	PB0		LED_BOARD
	//	PC14 	RELAY_1
	//	PC15 	RELAY_2
	//	18B20	1-GND;	2-DATA;	3-5V
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

		//char ds18b20_rom_2[8] = { 0x28, 0xFF, 0x1F, 0x4C, 0x23, 0x17, 0x03, 0xB9 } ;
		char ds18b20_rom_1[8] = { 0x28, 0xFF, 0xB0, 0x4E, 0x23, 0x17, 0x03, 0xE2 } ;	//	bad
		char ds18b20_rom_2[8] = { 0x28, 0xFF, 0x31, 0x50, 0x23, 0x17, 0x03, 0xC9 } ;
		char ds18b20_rom_3[8] = { 0x28, 0xFF, 0x55, 0x64, 0x4C, 0x04, 0x00, 0x20 } ;

		volatile uint8_t rtc_irq_u8	= 0;

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

	HAL_GPIO_WritePin ( LED_BOARD_GPIO_Port, LED_BOARD_Pin , GPIO_PIN_SET ) ;
	HAL_GPIO_WritePin ( RELAY_2_GPIO_Port  , RELAY_2_Pin   , GPIO_PIN_SET ) ;

	char WeekDay_char[7][4];
	sprintf(WeekDay_char[0],"SUN");
	sprintf(WeekDay_char[1],"MON");
	sprintf(WeekDay_char[2],"TUE");
	sprintf(WeekDay_char[3],"WED");
	sprintf(WeekDay_char[4],"THU");
	sprintf(WeekDay_char[5],"FRI");
	sprintf(WeekDay_char[6],"SAT");

	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\tThermoStat 2021-February-26 v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	LCD1602_Init(&h1_lcd1602_fc113);
//	LCD1602_scan_I2C_bus(&h1_lcd1602_fc113);
//	LCD1602_Clear(&h1_lcd1602_fc113);

	//DS18b20_Print_serial_number(&huart1);
	//Set_Day_and_Time_to_DS3231 (2020, 11, 23, 17, 22, 00);


	RTC_TimeTypeDef TimeSt = { 0 } ;
	RTC_DateTypeDef DateSt = { 0 } ;
	ds3231_GetTime ( ADR_I2C_DS3231, &TimeSt ) ;
	ds3231_GetDate ( ADR_I2C_DS3231, &DateSt ) ;
	HAL_RTC_SetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN );
	HAL_RTC_SetDate( &hrtc, &DateSt, RTC_FORMAT_BIN );

	ds3231_PrintTime		( &TimeSt, &huart1 ) ;
	ds3231_PrintWeek3char	( &DateSt, &huart1 ) ;
	ds3231_PrintDate		( &DateSt, &huart1 ) ;

		HAL_IWDG_Refresh(&hiwdg) ;
		//DS18b20_ConvertTemp_SkipROM();
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_1);
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_2);
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_3);
		HAL_Delay(1000);
		int temp1 = DS18b20_Get_temp_MatchROM(ds18b20_rom_1)/16;
		int temp2 = DS18b20_Get_temp_MatchROM(ds18b20_rom_2)/16;
		int temp3 = DS18b20_Get_temp_MatchROM(ds18b20_rom_3)/16;
		HAL_IWDG_Refresh(&hiwdg) ;

	sprintf(DataChar,"%02d:%02d:%02d %s\n",TimeSt.Hours, TimeSt.Minutes, TimeSt.Seconds, WeekDay_char[(DateSt.WeekDay+3)%6]);
	LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));

	sprintf(DataChar," %d; %d; %d;\r\n", temp1, temp2, temp3);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	sprintf(DataChar,"%d %d %d\r\n", temp1, temp2, temp3);
	LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));
	//DS18b20_Print_serial_number(&huart1);
	LCD1602_Cursor_Return(&h1_lcd1602_fc113);

	ds3231_Alarm1_SetEverySeconds(ADR_I2C_DS3231);
	//ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);
	ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
	//rtc_irq_u8 = 1;
}
//************************************************************************

void Thermostat_Main(void) {

	char DataChar[100];
	char WeekDay_char[7][4];
	sprintf(WeekDay_char[0],"SUN");
	sprintf(WeekDay_char[1],"MON");
	sprintf(WeekDay_char[2],"TUE");
	sprintf(WeekDay_char[3],"WED");
	sprintf(WeekDay_char[4],"THU");
	sprintf(WeekDay_char[5],"FRI");
	sprintf(WeekDay_char[6],"SAT");

	RTC_TimeTypeDef TimeSt = { 0 } ;
	RTC_DateTypeDef DateSt = { 0 } ;


	if 	(rtc_irq_u8 == 1) {
	ds3231_GetTime ( ADR_I2C_DS3231, &TimeSt ) ;
	ds3231_GetDate ( ADR_I2C_DS3231, &DateSt ) ;

	if (TimeSt.Seconds == 55) {
		//DS18b20_ConvertTemp_SkipROM();
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_1 ) ;
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_2 ) ;
		DS18b20_ConvertTemp_MatchROM(ds18b20_rom_3 ) ;
	}

	if (TimeSt.Seconds == 57) {
		Temp_str.ds18b20_i[0] = DS18b20_Get_temp_MatchROM(ds18b20_rom_1)/16 ;
		Temp_str.ds18b20_i[1] = DS18b20_Get_temp_MatchROM(ds18b20_rom_2)/16 ;
		Temp_str.ds18b20_i[2] = DS18b20_Get_temp_MatchROM(ds18b20_rom_3)/16 ;

		Temp_str.average_i =(	Temp_str.ds18b20_i[0]
							+	Temp_str.ds18b20_i[1]
							+	Temp_str.ds18b20_i[2]) / 3 ;
	}

	if (TimeSt.Seconds == 0) {
		static char relay_status_char[4] = { "nop" };
		static uint8_t relay_status_u8 = 0 ;

		if (Temp_str.average_i < TEMP_MIN )		{ relay_status_u8 = 1 ;	}
		if (Temp_str.average_i > TEMP_MAX )		{ relay_status_u8 = 0 ;	}

		if ( relay_status_u8 == 1 ) {
			sprintf(relay_status_char,"On ");
			HAL_GPIO_WritePin ( LED_BOARD_GPIO_Port, LED_BOARD_Pin , GPIO_PIN_RESET ) ;
			HAL_GPIO_WritePin ( RELAY_2_GPIO_Port  , RELAY_2_Pin   , GPIO_PIN_RESET ) ;
		}

		if (relay_status_u8 == 0 )
			{
			sprintf(relay_status_char,"Off");
			HAL_GPIO_WritePin ( LED_BOARD_GPIO_Port, LED_BOARD_Pin , GPIO_PIN_SET ) ;
			HAL_GPIO_WritePin ( RELAY_2_GPIO_Port  , RELAY_2_Pin   , GPIO_PIN_SET ) ;
			}

		ds3231_PrintTime		( &TimeSt, &huart1 ) ;
		ds3231_PrintWeek3char	( &DateSt, &huart1 ) ;
		ds3231_PrintDate		( &DateSt, &huart1 ) ;

		sprintf(DataChar, "%02d:%02d:%02d %s %d\n" , TimeSt.Hours, TimeSt.Minutes, TimeSt.Seconds, WeekDay_char[(DateSt.WeekDay+3)%6] ,relay_status_u8) ;
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));

		sprintf(DataChar,"%03d %03d %03d %04d\r\n", Temp_str.ds18b20_i[0]/10, Temp_str.ds18b20_i[1]/10, Temp_str.ds18b20_i[2]/10, Temp_str.average_i ) ;
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));
		//DS18b20_Print_serial_number(&huart1);
		LCD1602_Cursor_Return(&h1_lcd1602_fc113);

		sprintf(DataChar," %04d; %04d; %04d; average=%04d %s\r\n", Temp_str.ds18b20_i[0], Temp_str.ds18b20_i[1], Temp_str.ds18b20_i[2], Temp_str.average_i, relay_status_char ) ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	} else {
		sprintf(DataChar," %02d:%02d:%02d\r" , TimeSt.Hours, TimeSt.Minutes, TimeSt.Seconds ) ;
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		sprintf(DataChar, "%02d:%02d:%02d" , TimeSt.Hours, TimeSt.Minutes, TimeSt.Seconds ) ;
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));
		LCD1602_Cursor_Return(&h1_lcd1602_fc113);
	}

	HAL_IWDG_Refresh(&hiwdg) ;
	Reset_RTC_IRQ_Flag() ;
	ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231) ;
	}
}
//-------------------------------------------------------------------------------------------------

void Set_RTC_IRQ_Flag ( void ) {
	rtc_irq_u8 = 1;
}

//************************************************************************
void Reset_RTC_IRQ_Flag ( void ) {
	rtc_irq_u8 = 0;
}

//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/


//************************************************************************
