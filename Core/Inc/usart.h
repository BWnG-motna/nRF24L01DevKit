/*
 * usart.h
 *
 *  Created on: Feb 24, 2025
 *      Author: Daniel Lee
 */

#pragma once

#include "common.h"
#include "stm32f1xx_hal_uart.h"

#include "ringbuffer.h"


namespace daniel
{

class USART
{

private :
	UART_HandleTypeDef * pHandle ;
	RingBuffer< uint8_t > ringBuf ;

	bool    isBegin ;

	uint8_t UART_rxBuf ;

private :
	void Send_A_CH( uint32_t const & dat ) const ;
	void Stripping(
		uint8_t * prBuf , uint16_t & rcnt ,
		uint8_t const * const psBuf , uint16_t const & scnt ) const ;
	void Stuffing(
			uint8_t * prBuf , uint16_t & rcnt ,
			uint8_t const * const psBuf , uint16_t const & scnt ) const ;

public :
	void SetHandle( UART_HandleTypeDef * pHandle ) ;

public :
	void SendMessage( uint8_t const * const pDat , uint16_t const length ) const ;
	void SendMessage( char const * const format , ... ) const ;

public :
	void Recv_A_CH( uint8_t const ch ) ;

public :
	USART() ;
	USART( UART_HandleTypeDef * pHandle ) ;

	void Begin() ;



} ; // class USART

} // namespace daniel
