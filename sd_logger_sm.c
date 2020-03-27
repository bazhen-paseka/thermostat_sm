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
	#include "sd_logger_sm.h"
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
		FRESULT		fres;
		uint8_t		button_download_pressed = 0;
		uint8_t		time_count_update_flag = 0;
		uint32_t	second_count_u32 = 0;
		uint32_t 	counter_u32 = 0;

		RTC_TimeTypeDef TimeSt;
		RTC_DateTypeDef DateSt;

		lcd1602_fc113_struct h1_lcd1602_fc113 =
		{
			.i2c = &hi2c1,
			.device_i2c_address = ADR_I2C_FC113
		};

		int ds18b20_temp_int[2] ;

		#define SD_FILE_NAME_SIZE 			 15
		char sd_file_name_char[SD_FILE_NAME_SIZE];

		uint8_t dataOut[32];	/* Data received and data for send */
		uint8_t  dataIn[32];
		NRF24L01_Transmit_Status_t NRF24L01_Status;	/* NRF transmission status */
		char nrf24_string[32];

	#ifdef MASTER
		uint8_t MyAddress[] = { 0, 0, 0, 0, 0x10 };	/* My address */
		uint8_t Tx0Address[] = { 0, 0, 0, 0, 0x21 };	/* Other end address */
		uint8_t Tx1Address[] = { 0, 0, 0, 0, 0x22 };	/* Other end address */
	#endif
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

void SD_Logger_Init(void) {
	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\tSD-card logger 2020-march-22 v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	LCD1602_Init(&h1_lcd1602_fc113);
	I2C_ScanBus_to_LCD1602(&h1_lcd1602_fc113);

#if (SET_RTC_TIM_AND_DATE == 1)
	Set_Date_and_Time_to_DS3231(0x20, 0x03, 0x23, 0x15, 0x10, 0x00);
#endif

	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	ds3231_PrintTime(&TimeSt, &huart1);
	ds3231_PrintDate(&DateSt, &huart1);

	FATFS_SPI_Init(&hspi1);	/* Initialize SD Card low level SPI driver */

	HAL_IWDG_Refresh(&hiwdg);

	static uint8_t try_u8;
	do {
		fres = f_mount(&USERFatFS, "", 1);	/* try to mount SDCARD */
		if (fres == FR_OK) {
			sprintf(DataChar,"\r\nSDcard mount - Ok \r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}
		else {
			f_mount(NULL, "", 0);			/* Unmount SDCARD */
			Error_Handler();
			try_u8++;
			sprintf(DataChar,"%d)SDcard mount: Failed  Error: %d\r\n", try_u8, fres);
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			HAL_Delay(1000);
		}
	} while ((fres !=0) && (try_u8 < 3));

	LCD1602_Clear(&h1_lcd1602_fc113);

	NRF24L01_Init(&hspi2, MY_CHANNEL, 32);
	NRF24L01_SetRF(NRF24L01_DataRate_250k, NRF24L01_OutputPower_M6dBm);	/* Set 250kBps data rate and -6dBm output power */
	NRF24L01_SetMyAddress(MyAddress);	/* Set my address, 5 bytes */

	//ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);
	ds3231_Alarm1_SetEverySeconds(ADR_I2C_DS3231);
	ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);

	//HAL_TIM_Base_Start(&htim4);
	//HAL_TIM_Base_Start_IT(&htim4);
	HAL_IWDG_Refresh(&hiwdg);
}
//************************************************************************

void SD_Logger_Main(void) {
	char DataChar[100];
	char ds18b20_rom_1[8] = {0x28, 0xFF, 0x31, 0x50, 0x23, 0x17, 0x03, 0xC9};
	char ds18b20_rom_2[8] = {0x28, 0xFF, 0x55, 0x64, 0x4C, 0x04, 0x00, 0x20};

	if (time_count_update_flag == 1) {
		HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
		HAL_IWDG_Refresh(&hiwdg);

		time_count_update_flag  = 0;
		ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
		second_count_u32++;

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);			//ds3231_PrintTime(&TimeSt, &huart1);
		ds3231_GetDate(ADR_I2C_DS3231, &DateSt);			//ds3231_PrintDate(&DateSt, &huart1);

		LCD1602_Cursor_Return(&h1_lcd1602_fc113);
		sprintf( DataChar,"  %02d:%02d:%02d %02d,%02d", (int)TimeSt.Hours, (int)TimeSt.Minutes, (int)TimeSt.Seconds, ds18b20_temp_int[1]/100, ds18b20_temp_int[0]%100);
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));
		sprintf( nrf24_string, "%s", DataChar );

		//LCD1602_Cursor_Shift_Right(&h1_lcd1602_fc113, 2);
		sprintf(DataChar,"%04d/%02d/%02d %02d,%02d", (int)(DateSt.Year + 2000), (int)DateSt.Month, (int)DateSt.Date, ds18b20_temp_int[0]/100, ds18b20_temp_int[1]%100);
		LCD1602_Print_Line(&h1_lcd1602_fc113, DataChar, strlen(DataChar));

		if (second_count_u32 == SECOND-2) {
			DS18b20_ConvertTemp_MatchROM(ds18b20_rom_1);
			DS18b20_ConvertTemp_MatchROM(ds18b20_rom_2);
			//DS18b20_Print_serial_number(&huart1);
		}
	//-------------------------------------------------------------------------------------------------

		if (second_count_u32 == SECOND-1) {
			uint32_t sendTime_u32 = HAL_GetTick();
			NRF24L01_SetTxAddress(Tx0Address);	/* Set TX address, 5 bytes */
			//sprintf((char *) dataOut, "CurTemp=%d", ds18b20_temp_int[0]);
			sprintf((char *) dataOut, "%s", nrf24_string);
			sprintf(DataChar,"\r\nNRF24_tx: %s; ", dataOut);
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			/* Transmit data, goes automatically to TX mode */

			NRF24L01_Transmit(dataOut);
			do {				/* Wait for data to be sent */
				NRF24L01_Status = NRF24L01_GetTransmissionStatus();					/* Get transmission status */
			} while (NRF24L01_Status == NRF24L01_Transmit_Status_Sending);
			NRF24L01_PowerUpRx();	/* Go back to RX mode */

			sprintf(DataChar,"TX_time: %d\r\n", (int)(HAL_GetTick()-sendTime_u32) );
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			/* Wait received data, wait max 1000ms, if time is larger, then data were probably lost */
			while (!NRF24L01_DataReady() && (HAL_GetTick() - sendTime_u32) < 1000);
			NRF24L01_GetData(dataIn);				/* Get data from NRF2L01+ */

			sprintf(DataChar,"NRF24_rx: %s; ping: %d ms; ", dataIn, (int)(HAL_GetTick() - sendTime_u32));
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			if (NRF24L01_Status == NRF24L01_Transmit_Status_Ok) {	/* Check transmit status */
				sprintf(DataChar,"Status:OK\r\n\r\n");						/* Transmit went OK */
			} else if (NRF24L01_Status == NRF24L01_Transmit_Status_Lost) {
				sprintf(DataChar,"Status:LOST\r\n");					/* Message was LOST */
			} else {
				sprintf(DataChar,"Sending data: \r\n");					/* This should never happen */
			}
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}
	//-------------------------------------------------------------------------------------------------

		if (second_count_u32 >= SECOND) {
			second_count_u32  = 0;

			ds18b20_temp_int[0] = DS18b20_Get_temp_MatchROM(ds18b20_rom_1) / 16;
			//float temp_1_fl = (float)ds18b20_temp_int[0] / 100.0;
			ds18b20_temp_int[1] = DS18b20_Get_temp_MatchROM(ds18b20_rom_2) / 16;
			//float temp_2_fl = (float)de18b20_temp_int[1] / 100.0;
			//sprintf(DataChar,"%.03f(%d) %.03f(%d) ", temp_1_fl, ds18b20_temp_1_int/16, temp_2_fl, de18b20_temp_int[2]/16);
			//HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			snprintf(DataChar, 40,"%04d\t%02d\t%02d\t%02d\t%02d\t%02d\t%04d\t%04d\t%04d\r\n",
					(int)(DateSt.Year + 2000), (int)DateSt.Month, (int)DateSt.Date,
					(int)TimeSt.Hours, (int)TimeSt.Minutes, (int)TimeSt.Seconds,
					(int)(100000*DateSt.Year + 10000*DateSt.Month + 10000*DateSt.Date + 10000*TimeSt.Hours + 100*TimeSt.Minutes + TimeSt.Seconds),
					ds18b20_temp_int[0], ds18b20_temp_int[1]);
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

			sprintf(sd_file_name_char,"%02d%02d%02d%02d.txt",(int)DateSt.Month, (int)DateSt.Date, (int)TimeSt.Hours, (int)TimeSt.Minutes);		 // (int)DateSt.Year,
			int len = strlen(sd_file_name_char) + 1;
			char PathString[len];
			snprintf(PathString, len,"%s", sd_file_name_char);
			TCHAR *f_tmp = sd_file_name_char;
			char *s_tmp = PathString;
			while(*s_tmp) {
				*f_tmp++ = (TCHAR)*s_tmp++;
			}
			*f_tmp = 0;
			HAL_UART_Transmit(&huart1, (uint8_t *)sd_file_name_char, strlen(sd_file_name_char), 100);

			fres = f_open(&USERFile, sd_file_name_char, FA_OPEN_ALWAYS | FA_WRITE);
			fres += f_lseek(&USERFile, f_size(&USERFile));

			if (fres == FR_OK) 	{
				f_printf(&USERFile, "%s", DataChar);	/* Write to file */
				uint32_t file_size_u32 = f_size(&USERFile);
				f_close(&USERFile);	/* Close file */
				sprintf(DataChar," SD_wr-Ok; size=%d; ", (int)file_size_u32);
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			} else {
				sprintf(DataChar," SD_wr-Err; ");
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			}
		}
	}

	if (button_download_pressed == 1) {
		button_download_pressed =  0;
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, RESET);
		sprintf(DataChar,"Download data to port...\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		fres = f_open(&USERFile, sd_file_name_char, FA_OPEN_EXISTING | FA_READ);
		if (fres == FR_OK) {
			sprintf(DataChar,"FR - Ok;\r\n");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			char buff[200];
			while (f_gets(buff, 200, &USERFile)) {
				sprintf(DataChar,"%s\r",buff);
				HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
			}
			f_close(&USERFile);
		} else {
			sprintf(DataChar,"\tFR - Fail;");
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}

		HAL_Delay(999);
		sprintf(DataChar,"\r\n\t...download finish.\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, SET);
	}
}
//************************************************************************

void Set_button_download_pressed(uint8_t _new_button_status_u8) {
	button_download_pressed = _new_button_status_u8;
}
//************************************************************************

void Set_time_count_update_flag(uint8_t _new_flag_u8) {
	time_count_update_flag = _new_flag_u8;
}
//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

//************************************************************************
