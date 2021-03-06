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

#ifndef THERMOSTAT_SM_H_INCLUDED
#define THERMOSTAT_SM_H_INCLUDED

/*
**************************************************************************
*								INCLUDE FILES
**************************************************************************
*/
	#include "stdio.h"
	#include <string.h>

	#include "main.h"
	#include "stm32f1xx_hal.h"
	#include "gpio.h"
	#include "usart.h"
	#include "iwdg.h"

	#include "thermostat_config.h"
	#include "i2c_techmaker_sm.h"
	#include "lcd1602_fc113_sm.h"
	#include "ds18b20_sm.h"
	#include "ds3231_sm.h"

#include "rtc.h"


/*
**************************************************************************
*								    DEFINES
**************************************************************************
*/

	#define DS18B20QNT	3

/*
**************************************************************************
*								   DATA TYPES
**************************************************************************
*/

	typedef struct {
		int	ds18b20_i[DS18B20QNT]	;
		int average_i	;
	} TEMP_18B20_struct;

	//---------------------------------------------------------------------

	TEMP_18B20_struct Temp_str;


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
	void Thermostat_Init  ( void ) ;
	void Thermostat_Main  ( void ) ;
	void Set_RTC_IRQ_Flag ( void ) ;
	void Reset_RTC_IRQ_Flag ( void ) ;

#endif /* THERMOSTAT_SM_H_INCLUDED */
