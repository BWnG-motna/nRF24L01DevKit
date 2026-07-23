/*
 * mainProc.cpp
 *
 *  Created on: Feb 24, 2025
 *      Author: Daniel Lee
 */


#include "mainProc.h"

#include "GPIO.h"
#include "usart.h"
#include "crc16.h"
#include "nRF24L01.h"

extern UART_HandleTypeDef huart1 ;
extern UART_HandleTypeDef huart3 ;
extern UART_HandleTypeDef huart4 ;

extern TIM_HandleTypeDef  htim6 ;
extern SPI_HandleTypeDef  hspi1 ;

daniel::USART    uart1( & huart1 ) ; // on USB
daniel::USART    uart3( & huart3 ) ; // pin header H1
daniel::USART    uart4( & huart4 ) ; // pin header H2
daniel::nRF24L01 rf( & hspi1 ) ;

volatile static bool oneSecIrq = false ;
volatile static bool liveIrq   = false ;
volatile static bool runOp     = false ;


#define DIPSW_IS_EXISTED ( 0 )


void MainProc()
{
	uart1.Begin() ;
	uart1.SendMessage( "\r\n\r\n" ) ;

#if ( DIPSW_IS_EXISTED )

	bool is = daniel::GPIO::GetOpMode() ;
	daniel::RfMode rfMode = ( true == is ) ? daniel::RfMode::TX : daniel::RfMode::RX ;

#else

	daniel::RfMode rfMode = daniel::RfMode::TX ;

#endif

	rf.LeaveLog( true ) ;
	rf.SetUart( & uart1 ) ;
	rf.Begin( rfMode ) ;

	HAL_TIM_Base_Start_IT( & htim6 ) ;

	daniel::GPIO::SetEventLed ( false ) ;

	/**/ if( daniel::RfMode::RX == rfMode )
	{
		daniel::GPIO::SetRxModeLed( true  ) ;
		daniel::GPIO::SetTxModeLed( false ) ;
	}
	else if( daniel::RfMode::TX == rfMode )
	{
		daniel::GPIO::SetRxModeLed( false ) ;
		daniel::GPIO::SetTxModeLed( true  ) ;
	}
	else
	{
		daniel::GPIO::SetRxModeLed( false ) ;
		daniel::GPIO::SetTxModeLed( false ) ;
	}

	uint8_t payload[ 32 ] ;
	for( uint8_t pos = 0 ; pos < 32 ; ++pos )
	{
		payload[ pos ] = pos ;
	}

	runOp = true ;

	while( true )
	{
		rfMode = rf.GetRfMode() ;

		if( true == oneSecIrq )
		{
			if( daniel::RfMode::TX == rfMode )
			{
				rf.Transmit( payload , 32 ) ;
			}
			else
			{
				rf.ShowSpecificValue() ;
			}
			oneSecIrq = false ;
		}

		if( true == liveIrq )
		{
			if( daniel::RfMode::TX == rfMode )
			{
				daniel::GPIO::ToggleTxModeLed() ;
			}
			else
			{
				daniel::GPIO::ToggleRxModeLed() ;
			}

			liveIrq = false ;
		}
	}
}


void UartRX( UART_HandleTypeDef * pHandle , uint8_t const port )
{
	if( false == runOp )
	{
		return ;
	}

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
	if( false == runOp )
	{
		return ;
	}

	/**/ if( 1 == port )
	{

	}
	else if( 3 == port )
	{

	}
	else if( 4 == port )
	{

	}
}


void TimIrq() // is called each 1 milliseconds
{
	if( false == runOp )
	{
		return ;
	}

	static uint16_t upCount = 0 ;

	++upCount ;

	if( ( 0 != upCount ) && ( 0 == upCount % 250 ) )
	{
		liveIrq = true ;
	}

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
		daniel::GPIO::ToggleEventLed() ;
	}
}
