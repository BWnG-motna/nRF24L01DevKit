#pragma once

#include "common.h"

namespace daniel
{

enum class RfMode : uint8_t
{
	TX      = 0x00 ,
	RX      = 0x01 ,
	Unknown = 0xFF ,
} ;

uint8_t ConvRfModeToUint( RfMode const & mode ) ;

} // namespace daniel
