/*
 * LEDs.h
 *
 *  Created on: Sep 15, 2025
 *      Author: yukinpl
 */

#pragma once

#include "common.h"

namespace daniel
{

class GPIO
{

public :
	static void SetEventLed( bool const & isOn ) ;
	static void SetTxModeLed( bool const & isOn ) ;
	static void SetRxModeLed( bool const & isOn ) ;

	static void ToggleEventLed() ;
	static void ToggleTxModeLed() ;
	static void ToggleRxModeLed() ;

	static bool GetOpMode() ;

} ;

} // daniel
