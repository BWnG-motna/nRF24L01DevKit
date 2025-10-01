/*
 * LEDs.cpp
 *
 *  Created on: Sep 15, 2025
 *      Author: yukinpl
 */

#include "LEDs.h"


void daniel::LEDs::SetDebugLed1( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_3 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::LEDs::SetDebugLed2( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_2 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::LEDs::SetDebugLed3( bool const & isOn )
{
	HAL_GPIO_WritePin( GPIOA , GPIO_PIN_1 ,  ( true == isOn ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::LEDs::ToggleDebugLed1()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_3 ) ;
}


void daniel::LEDs::ToggleDebugLed2()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_2 ) ;
}


void daniel::LEDs::ToggleDebugLed3()
{
	HAL_GPIO_TogglePin( GPIOA , GPIO_PIN_1 ) ;
}
