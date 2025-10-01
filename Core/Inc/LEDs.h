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

class LEDs
{

public :
	static void SetDebugLed1( bool const & isOn ) ;
	static void SetDebugLed2( bool const & isOn ) ;
	static void SetDebugLed3( bool const & isOn ) ;

	static void ToggleDebugLed1() ;
	static void ToggleDebugLed2() ;
	static void ToggleDebugLed3() ;

} ;

} // daniel
