/*
 * mainProc.h
 *
 *  Created on: Feb 24, 2025
 *      Author: Daniel Lee
 */

#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include "common.h"

void MainProc() ;
void UartRX( UART_HandleTypeDef * pHandle , uint8_t const port ) ;
void ReceiveUartRx( uint8_t const dat , uint8_t const port ) ;

void TimIrq() ;


#ifdef __cplusplus
}
#endif

