/*
 * RfAutoRetransmitDelay.h
 *
 *  Created on: Jul 23, 2026
 *      Author: yukinpl
 */

#pragma once

#include "common.h"


namespace daniel
{


enum class RfARD : uint8_t  // auto retransmit delay
{
	Delay_250us    =  0 ,
	Delay_500us    =  1 ,
	Delay_750us    =  2 ,
	Delay_1000us   =  3 ,
	Delay_1250us   =  4 ,
	Delay_1500us   =  5 ,
	Delay_1750us   =  6 ,
	Delay_2000us   =  7 ,
	Delay_2250us   =  8 ,
	Delay_2500us   =  9 ,
	Delay_2750us   = 10 ,
	Delay_3000us   = 11 ,
	Delay_3250us   = 12 ,
	Delay_3500us   = 13 ,
	Delay_3750us   = 14 ,
	Delay_4000us   = 15 ,

} ;


} // namespace daniel
