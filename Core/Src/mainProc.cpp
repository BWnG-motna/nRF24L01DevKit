/*
 * mainProc.cpp
 *
 *  Created on: Feb 24, 2025
 *      Author: Daniel Lee
 */


#include "mainProc.h"
#include "usart.h"
#include "crc16.h"
#include "nRF24L01.h"

extern UART_HandleTypeDef huart1 ;
extern TIM_HandleTypeDef  htim6 ;
extern SPI_HandleTypeDef  hspi1 ;

daniel::USART    uart( & huart1 ) ;
daniel::nRF24L01 rf( & hspi1 ) ;

daniel::RfMode   rfMode = daniel::RfMode::TX ;


static bool oneSecIrq = false ;


void MainProc()
{
	uart.Begin() ;
	uart.SendMessage( "\r\n\r\n" ) ;

	rf.SetUart( & uart ) ;
	rf.Begin( rfMode ) ;

	HAL_TIM_Base_Start_IT( &htim6 ) ;

	uint8_t payload[ 32 ] ;
	for( uint8_t pos = 0 ; pos < 32 ; ++pos )
	{
		payload[ pos ] = pos ;
	}

	while( true )
	{
		if( true == oneSecIrq )
		{
			if( daniel::RfMode::TX == rfMode )
			{
				rf.Transmit( payload ) ;
			}
			oneSecIrq = false ;
		}
	}
}


void UartRX( UART_HandleTypeDef * pHandle , uint8_t const port )
{
	uint32_t isrflags = READ_REG( pHandle->Instance->SR  ) ;
	uint32_t cr1its   = READ_REG( pHandle->Instance->CR1 ) ;

	if( ( 0U != ( isrflags & USART_SR_RXNE ) ) && ( 0U != ( cr1its & USART_CR1_RXNEIE ) ) )
	{
		uint32_t rx = pHandle->Instance->DR ;
		uint8_t dat = ( uint8_t )( rx ) ;

		ReceiveUartRx( dat , port ) ;
	}
}


void ReceiveUartRx( uint8_t const dat , uint8_t const port )
{
	if( 1 == port )
	{

	}
}


void TimIrq() // is called each 10 milliseconds
{
	static uint16_t upCount = 0 ;

	++upCount ;
	if( 1000 <= upCount )
	{
		upCount = 0 ;
		oneSecIrq = true ;
	}
}


void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	if( GPIO_Pin == daniel::nRF24L01::IRQ_Pin )
	{
		rf.Irq() ;
	}
}
