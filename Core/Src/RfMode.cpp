#include "RfMode.h"


uint8_t daniel::ConvRfModeToUint( RfMode const & mode )
{
	uint8_t res = 0 ;

	switch( mode )
	{
		case RfMode::RX :
			res = 0x01 ;
			break ;

		case RfMode::TX :
			res = 0x00 ;
			break ;

		default :
			res = 0xFF ;
			break ;
	}

	return res ;
}
