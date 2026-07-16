/*
 * LEDs.cpp
 *
 *  Created on: Sep 15, 2025
 *      Author: yukinpl
 */

#include <GPIO.h>


void daniel::GPIO::SetEventLed( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_3 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::GPIO::SetTxModeLed( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_2 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::GPIO::SetRxModeLed( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_1 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::GPIO::ToggleEventLed()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_3 ) ;
}


void daniel::GPIO::ToggleTxModeLed()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_2 ) ;
}


void daniel::GPIO::ToggleRxModeLed()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_1 ) ;
}


bool daniel::GPIO::GetOpMode()
{
	GPIO_PinState pinState = HAL_GPIO_ReadPin( GPIOC , GPIO_PIN_0 ) ;

	return ( GPIO_PIN_SET == pinState ) ? true : false ;
}
