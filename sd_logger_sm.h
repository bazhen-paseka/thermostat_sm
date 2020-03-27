/**
* \file
* \version 1.0
* \author bazhen.levkovets
* \date 2018 
* \mail bazhen.info(at)gmail.com
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*
*************************************************************************************
*
* \brief
*
*/

#ifndef SD_LOGGER_SM_H_INCLUDED
#define SD_LOGGER_SM_H_INCLUDED

/*
**************************************************************************
*								INCLUDE FILES
**************************************************************************
*/
	#include "stm32f1xx_hal.h"
	#include "gpio.h"
	#include "usart.h"
	#include "stdio.h"
	#include <string.h>
	#include "fatfs.h"
	#include "spi.h"
	#include "SD_Logger_f103_config.h"
	//#include "tim.h"
	//#include "ringbuffer_dma.h"
	#include "ds3231_sm.h"
	#include "i2c.h"
	#include "rtc.h"
	#include "iwdg.h"
	#include "ds18b20_sm.h"
	#include "lcd1602_fc113_sm.h"
	#include "NRF24L01_sm.h"

/*
**************************************************************************
*								    DEFINES
**************************************************************************
*/

/*
**************************************************************************
*								   DATA TYPES
**************************************************************************
*/

/*
**************************************************************************
*								GLOBAL VARIABLES
**************************************************************************
*/

/*
**************************************************************************
*									 MACRO'S
**************************************************************************
*/

/*
**************************************************************************
*                              FUNCTION PROTOTYPES
**************************************************************************
*/
	void SD_Logger_Init(void);
	void SD_Logger_Main(void);
	void Set_button_download_pressed(uint8_t _new_button_status_u8);
	void Set_time_count_update_flag(uint8_t _new_flag_u8);
#endif /* SD_LOGGER_SM_H_INCLUDED */
