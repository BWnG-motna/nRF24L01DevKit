/*
 * RfPower.h
 *
 *  Created on: Jul 16, 2026
 *      Author: yukinpl
 */

#pragma once

#include "common.h"


namespace daniel
{


enum class RfPower : uint8_t
{
	Power_Neg18dBm  =  0 ,
	Power_Neg12dBm  =  1 ,
	Power_Neg6dBm   =  2 ,
	Power_0dBm      =  3 ,

} ;


} // namespace daniel
