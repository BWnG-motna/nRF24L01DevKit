/*
 * usart.cpp
 *
 *  Created on: Feb 24, 2025
 *      Author: Daniel Lee
 */



#include "usart.h"
#include "crc16.h"

#include <stdio.h>  // vsprintf()
#include <stdarg.h> // va_list , va_start() , va_end()
#include <string.h> // for strlen()



daniel::USART::USART() :
	pHandle( nullptr ) , isBegin( false )
{
	ringBuf.Alloc( 50 ) ;
}


daniel::USART::USART( UART_HandleTypeDef * _pHandle ) :
	pHandle( _pHandle ) , isBegin( false )
{
	ringBuf.Alloc( 50 ) ;
}


void daniel::USART::SetHandle( UART_HandleTypeDef * _pHandle )
{
	pHandle = _pHandle ;
}


void daniel::USART::Begin()
{
	HAL_UART_DeInit( pHandle ) ;

	pHandle->Init.BaudRate     = 115200 ;
	pHandle->Init.WordLength   = UART_WORDLENGTH_8B ;
	pHandle->Init.StopBits     = UART_STOPBITS_1    ;
	pHandle->Init.Parity       = UART_PARITY_NONE   ;
	pHandle->Init.Mode         = UART_MODE_TX_RX     ;
	pHandle->Init.HwFlowCtl    = UART_HWCONTROL_NONE ;
	pHandle->Init.OverSampling = UART_OVERSAMPLING_16 ;

	HAL_UART_Init( pHandle ) ;

	pHandle->Instance->CR1 &= 0xFFFFBE3F ;
	pHandle->Instance->CR3 &= 0xFFFFFFFE ;

	HAL_UART_Receive_IT( pHandle , & UART_rxBuf , 1 ) ;

	isBegin = true ;
}


void daniel::USART::Send_A_CH( uint32_t const & dat ) const
{
	if( nullptr == pHandle || false == isBegin )
	{
		return ;
	}

	uint8_t isEnable = ( 0 < ( pHandle->Instance->CR1 & USART_CR1_UE ) ) ? 0x01 : 0x00 ;
	if( 0x00 == isEnable )
	{
		return ;
	}

	while ( ! ( __HAL_UART_GET_FLAG( pHandle , UART_FLAG_TXE ) ) ) ;
	pHandle->Instance->DR = dat ;
}


void daniel::USART::SendMessage( uint8_t const * const pDat , uint16_t const length ) const
{
	if( nullptr == pHandle || false == isBegin )
	{
		return ;
	}

	uint8_t  buf[ 512 ] ;
	uint16_t len ;

	Stuffing( buf , len , pDat , length ) ;

	for( uint16_t pos = 0 ; pos < len ; ++pos )
	{
		Send_A_CH( buf[ pos ] ) ;
	}
}


void daniel::USART::SendMessage( char const * const format , ... ) const
{
	if( nullptr == pHandle || false == isBegin )
	{
		return ;
	}

	static char szBuf[ 1024 ] ;

    va_list args ;
	va_start( args , format ) ;
	vsprintf( szBuf , format , args ) ;

    SendMessage( ( uint8_t * ) szBuf , strlen( szBuf ) ) ;

	va_end( args ) ;
}


void daniel::USART::Recv_A_CH( uint8_t const ch )
{
	if( nullptr == pHandle || false == isBegin )
	{
		return ;
	}
}


void daniel::USART::Stripping(
		uint8_t * prBuf , uint16_t & rcnt ,
		uint8_t const * const psBuf , uint16_t const & scnt ) const
{
	rcnt = 0 ;

	if( 0x7e == psBuf[ 0 ] && 0x7f == psBuf[ scnt - 1 ] )
	{
		for( uint16_t pos = 0 ; pos < scnt ; ++pos )
		{
			uint8_t dat = psBuf[ pos ] ;
			if( 0 != pos && ( scnt - 1 ) != pos && ( 0x7D == dat || 0x7E == dat || 0x7F == dat ) )
			{
				++pos ;
				dat = psBuf[ pos ] ^ 0x20 ;
			}

			prBuf[ rcnt++ ] = dat ;
		}
	}
	else
	{
		for( uint16_t pos = 0 ; pos < scnt ; ++pos )
		{
			prBuf[ pos ] = psBuf[ pos ] ;
		}

		rcnt = scnt ;
	}
}


void daniel::USART::Stuffing(
		uint8_t * prBuf , uint16_t & rcnt ,
		uint8_t const * const psBuf , uint16_t const & scnt ) const
{
	rcnt = 0 ;

	if( 0x7e == psBuf[ 0 ] && 0x7F == psBuf[ scnt - 1 ] )
	{
		for( uint16_t pos = 0 ; pos < scnt ; ++pos )
		{
			uint8_t dat = psBuf[ pos ] ;

			if( 0 != pos && ( scnt - 1 ) != pos && ( 0x7D == dat || 0x7E == dat || 0x7F == dat ) )
			{
				dat = dat ^ 0x20 ;
				prBuf[ rcnt++ ] = 0x7D ;
			}
			prBuf[ rcnt++ ] = dat ;
		}
	}
	else
	{
		for( uint16_t pos = 0 ; pos < scnt ; ++pos )
		{
			prBuf[ pos ] = psBuf[ pos ] ;
		}

		rcnt = scnt ;
	}
}
