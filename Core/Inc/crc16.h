/*
 * crc16.h
 *
 *  Created on: Feb 25, 2025
 *      Author: Daniel Lee
 */

#pragma once

#include "common.h"

namespace daniel
{

class CRC16
{

private :
	static uint16_t crc16XmodelTbl[ 256 ] ;

public :
	static uint16_t ComputeCRC16( uint8_t const * pDat , uint16_t const & length ) ;
} ;

}
