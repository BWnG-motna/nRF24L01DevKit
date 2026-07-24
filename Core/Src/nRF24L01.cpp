#include "nRF24L01.h"

#include "nordic.cmd.h"
#include "nordic.reg.h"
#include "nordic.type.h"

#include <stdio.h>  // vsprintf()
#include <stdarg.h> // va_list , va_start() , va_end()
#include <string.h> // for strlen()


 GPIO_TypeDef * daniel::nRF24L01::IRQ_Port = reinterpret_cast< GPIO_TypeDef * >( GPIOB_BASE ) ;
   char const * daniel::nRF24L01::errType [ 4 ] = { "HAL_OK" , "HAL_ERROR" , "HAL_BUSY" , "HAL_TIMEOUT" } ;
uint8_t const   daniel::nRF24L01::rfAddrP0[ 5 ] = { 0x13 , 0xA3 , 0xB3 , 0xCD , 0xEE } ;
uint8_t const   daniel::nRF24L01::rfAddrP1[ 5 ] = { 0x13 , 0xA3 , 0xB3 , 0xCD , 0xEF } ;
uint8_t const   daniel::nRF24L01::rfAddrP2      =  rfAddrP1[ 4 ] + 1 ;
uint8_t const   daniel::nRF24L01::rfAddrP3      =  rfAddrP1[ 4 ] + 2 ;
uint8_t const   daniel::nRF24L01::rfAddrP4      =  rfAddrP1[ 4 ] + 3 ;
uint8_t const   daniel::nRF24L01::rfAddrP5      =  rfAddrP1[ 4 ] + 4 ;


daniel::nRF24L01::nRF24L01( SPI_HandleTypeDef * _pHandle , bool const & _leaveLog )
	: pHandle( _pHandle )         , isCS( false )     , isCE( false )    , pUart( nullptr )    , payloadSize( 32 ) ,
	  leaveLog( _leaveLog )       , debugLog( false ) , autoACK( true )  , dynPayload( false ) , rfChannel( 0x4C ) ,
	  rfARD( RfARD::Delay_250us ) , rfMode( RfMode::Unknown ) , rfPower( RfPower::Power_0dBm ) , rfLnaGain( RfLnaGain::High ) , rfDataRate( RfDataRate::Rate_1Mbps ) ,
      rfARC( 3 )
{
	SetCS( false ) ;

	for( uint8_t pos = 0 ; pos < RfPipeCnt ; ++pos )
	{
		rfPipe[ pos ] = false ;
	}

	rfPipe[ 0 ] = true ;

	LogEvent( "nRF24L01: created\r\n" ) ;
}


daniel::nRF24L01::nRF24L01( SPI_HandleTypeDef * _pHandle , USART * _pUart , bool const & _leaveLog )
	: pHandle( _pHandle )         , isCS( false )     , isCE( false )    , pUart( _pUart )     , payloadSize( 32 ) ,
	  leaveLog( _leaveLog )       , debugLog( false ) , autoACK( true )  , dynPayload( false ) , rfChannel( 0x4C ) ,
	  rfARD( RfARD::Delay_250us ) , rfMode( RfMode::Unknown ) , rfPower( RfPower::Power_0dBm ) , rfLnaGain( RfLnaGain::High ) , rfDataRate( RfDataRate::Rate_1Mbps ) ,
	  rfARC( 3 )
{
	SetCS( false ) ;

	for( uint8_t pos = 0 ; pos < RfPipeCnt ; ++pos )
	{
		rfPipe[ pos ] = false ;
	}

	rfPipe[ 0 ] = true ;

	LogEvent( "nRF24L01: created\r\n" ) ;
}


void daniel::nRF24L01::SetCS( bool const & isEnable )
{
	// CSN signal
	// CSN must be enabled ( low ) during SPI communication
	isCS = isEnable ;
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_5 , ( true == isEnable ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::nRF24L01::SetCE( bool const & isEnable )
{
	// CE signal
	// RX mode - CE must be always enabled ( high )
	// TX mode - CE must be enabled during sending data ( pulse or hold ) , otherwise, must be disabled
	isCE = isEnable ;
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_4 , ( true == isEnable ) ? GPIO_PIN_SET : GPIO_PIN_RESET ) ;
}


void daniel::nRF24L01::Begin( RfMode const & mode )
{
	LogEvent( "nRF24L01: Begin\r\n" ) ;

	Init() ;
	SetRfMode( mode ) ;
	PowerOnOff( nordic::type::ON ) ;
	//Inspection() ;

	if( RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


void daniel::nRF24L01::End()
{
	LogEvent( "nRF24L01: End\r\n" ) ;

	PowerOnOff( nordic::type::OFF ) ;
}


void daniel::nRF24L01::SetChannel( uint8_t const & ch )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;

	rfChannel = ( 125 < ch ) ? 125 : ch ;

	bool wasCE = isCE ;

	SetCE( false ) ;
	AccessReg( TYPE::WRITE , REG::RF_CH , rfChannel ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


bool daniel::nRF24L01::Scan( bool channel[ 126 ] )
{
	if( RfMode::RX != rfMode )
	{
		return false ;
	}

	uint8_t wasRfChannel = rfChannel ;
	SetCE( false ) ;

	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	for( uint8_t pos = 0 ; pos < 126 ; ++pos )
	{
		AccessReg( TYPE::WRITE , REG::RF_CH , pos ) ;
		DelayUS( 200 ) ;

		SetCE( true  ) ;
		DelayUS( 200 ) ;
		SetCE( false ) ;

		uint8_t val = 0x00 ;
		uint8_t res = AccessReg( TYPE::READ , REG::RPD , val ) ;

		channel[ pos ] = ( 0 < ( 0x01 & res ) ) ? true : false ;
	}

	SetCE( true ) ;

	FlushFIFO( TYPE::RX ) ;
	SetChannel( wasRfChannel ) ;

	return true ;
}


void daniel::nRF24L01::SetARD( RfARD const & ard )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;

	rfARD = ard ;

	bool wasCE = isCE ;

	uint8_t setupRetrVal = GetSetupRetr() ;

	SetCE( false ) ;
	AccessReg( TYPE::WRITE , REG::SETUP_RETR , setupRetrVal ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


void daniel::nRF24L01::SetARC( uint8_t const & arc )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;

	rfARC = ( 15 < arc ) ? 15 : arc ;

	bool wasCE = isCE ;

	uint8_t setupRetrVal = GetSetupRetr() ;

	SetCE( false ) ;
	AccessReg( TYPE::WRITE , REG::SETUP_RETR , setupRetrVal ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}

void daniel::nRF24L01::SetRxPipe( uint8_t const & pipeNo , bool const & is )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;


	if( 0 == pipeNo || RfPipeCnt <= pipeNo )
	{
		return ;
	}

	rfPipe[ pipeNo ] = is ;

	bool wasCE = isCE ;

	uint8_t enAA     = GetEnAA() ;
	uint8_t enRxAddr = GetEnRxAddr() ;
	uint8_t dynPd    = GetDynPd() ;

	SetCE( false ) ;
	AccessReg( TYPE::WRITE , REG::EN_AA     , enAA     ) ; // enable AUTOACK
	AccessReg( TYPE::WRITE , REG::EN_RXADDR , enRxAddr ) ; // enable RX address for all data pipe
	AccessReg( TYPE::WRITE , REG::DYNPD     , dynPd    ) ; // Dynamic Payload

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}

void daniel::nRF24L01::SetRxPipe1( bool const & is )
{
	SetRxPipe( 1 , is ) ;
}


void daniel::nRF24L01::SetRxPipe2( bool const & is )
{
	SetRxPipe( 2 , is ) ;
}


void daniel::nRF24L01::SetRxPipe3( bool const & is )
{
	SetRxPipe( 3 , is ) ;
}


void daniel::nRF24L01::SetRxPipe4( bool const & is )
{
	SetRxPipe( 4 , is ) ;
}


void daniel::nRF24L01::SetRxPipe5( bool const & is )
{
	SetRxPipe( 5 , is ) ;
}


void daniel::nRF24L01::SetDynamicPayload()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;

	bool wasCE = isCE ;

	dynPayload = true ;

	uint8_t dynPd   = GetDynPd() ;
	uint8_t feature = 0x04 ;

	SetCE( false ) ;

	AccessReg( TYPE::WRITE , REG::DYNPD   , dynPd   ) ;
	AccessReg( TYPE::WRITE , REG::FEATURE , feature ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


void daniel::nRF24L01::SetStaticPayload( uint8_t const & length )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg ;

	bool wasCE = isCE ;

	dynPayload  = false ;
	payloadSize = ( 0 == length || 32 < length ) ? 32 : length ;

	uint8_t dynPd   = GetDynPd() ;
	uint8_t feature = 0x00 ;

	SetCE( false ) ;

	AccessReg( TYPE::WRITE , REG::DYNPD    , dynPd   ) ;
	AccessReg( TYPE::WRITE , REG::FEATURE  , feature ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P0 , payloadSize ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P1 , payloadSize ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P2 , payloadSize ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P3 , payloadSize ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P4 , payloadSize ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P5 , payloadSize ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


void daniel::nRF24L01::Init()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;


	LogEvent( "nRF24L01: Init\r\n" ) ;
	SetCS( true ) ;
	SetCE( true ) ;

	uint8_t enAA      = GetEnAA() ;
	uint8_t enRxAddr  = GetEnRxAddr() ;
	uint8_t setupRetr = GetSetupRetr() ;
	uint8_t rfSetup   = GetRfSetupVal() ;
	uint8_t ch        = ( 125 < rfChannel ) ? 125 : rfChannel ;
	uint8_t dynPd     = GetDynPd() ;
	uint8_t feature   = ( true == dynPayload ) ? 0x04 : 0x00 ;

	AccessReg( TYPE::WRITE , REG::CONFIG      , 0x08 ) ; // enable CRC
	AccessReg( TYPE::WRITE , REG::EN_AA       , enAA ) ; // enable AUTOACK
	AccessReg( TYPE::WRITE , REG::EN_RXADDR   , enRxAddr  ) ; // enable RX address for all data pipe
	AccessReg( TYPE::WRITE , REG::SETUP_AW    , 0x03      ) ; // address field width( 5 )
	AccessReg( TYPE::WRITE , REG::SETUP_RETR  , setupRetr ) ; // auto retransmit delay , retransmit count
	AccessReg( TYPE::WRITE , REG::RF_CH       , ch        ) ; // RF channel
	AccessReg( TYPE::WRITE , REG::RF_SETUP    , rfSetup   ) ; // data rates and RF output power
	AccessReg( TYPE::WRITE , REG::RF_STATUS   , 0x7F      ) ; // interrupt enabled for Rx and TX , RX FIFO empty, TX FIFO ready
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P0  , 0x05 ,   rfAddrP0 ) ; // Rx Address - pipe 0
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P1  , 0x05 ,   rfAddrP1 ) ; // Rx Address - pipe 1
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P2  , 0x01 , & rfAddrP2 ) ; // Rx Address - pipe 2
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P3  , 0x01 , & rfAddrP3 ) ; // Rx Address - pipe 3
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P4  , 0x01 , & rfAddrP4 ) ; // Rx Address - pipe 4
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P5  , 0x01 , & rfAddrP5 ) ; // Rx Address - pipe 5
	AccessReg( TYPE::WRITE , REG::TX_ADDR     , 0x05 ,   rfAddrP0 ) ; // Tx Address
	AccessReg( TYPE::WRITE , REG::RX_PW_P0    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P1    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P2    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P3    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P4    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P5    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::FIFO_STATUS , 0x11    ) ; // RX and TX FIFO( empty )
	AccessReg( TYPE::WRITE , REG::DYNPD       , dynPd   ) ; // Dynamic Payload
	AccessReg( TYPE::WRITE , REG::FEATURE     , feature ) ; // Others

	FlushFIFO() ;

	SetCE( false ) ;
}


uint8_t daniel::nRF24L01::GetEnAA() const
{
	uint8_t res = 0x00 ;

	for( uint8_t pos = 0 ; pos < RfPipeCnt ; ++pos )
	{
		res |= ( true == rfPipe[ pos ] ) ? ( 0x01 << pos ) : 0x00 ;
	}

	return res ;
}


uint8_t daniel::nRF24L01::GetEnRxAddr() const
{
	uint8_t res = 0x00 ;

	for( uint8_t pos = 0 ; pos < RfPipeCnt ; ++pos )
	{
		res |= ( true == rfPipe[ pos ] ) ? ( 0x01 << pos ) : 0x00 ;
	}

	return res ;
}


uint8_t daniel::nRF24L01::GetSetupRetr() const
{
	uint8_t ard = static_cast< uint8_t >( rfARD ) ;
	uint8_t arc = ( 15 < rfARC ) ? 15 : rfARC ;

	uint8_t res
		= ( ( ard << 4 ) & 0xF0 )
		| ( ( arc << 0 ) & 0x0F ) ;

	return res ;
}


uint8_t daniel::nRF24L01::GetDynPd() const
{
	uint8_t res = 0x00 ;


	if( false == dynPayload )
	{
		return res ;
	}

	for( uint8_t pos = 0 ; pos < RfPipeCnt ; ++pos )
	{
		res |= ( true == rfPipe[ pos ] ) ? ( 0x01 << pos ) : 0x00 ;
	}

	return res ;
}


uint8_t daniel::nRF24L01::GetRfSetupVal() const
{
	uint8_t val = 0x00 ;

	val |= ( RfDataRate::Rate_2Mbps  == rfDataRate ) ? 0x08 : 0x00 ;
	val |= ( RfPower::Power_Neg12dBm == rfPower    ) ? 0x02 : 0x00 ;
	val |= ( RfPower::Power_Neg6dBm  == rfPower    ) ? 0x04 : 0x00 ;
	val |= ( RfPower::Power_0dBm     == rfPower    ) ? 0x06 : 0x00 ;
	val |= ( RfLnaGain::High         == rfLnaGain  ) ? 0x01 : 0x00 ;

	return val ;
}


void daniel::nRF24L01::Inspection()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t value[ 23 ] ;
	uint8_t valuePos = 0 ;

	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::CONFIG      ) ; // CRC
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::EN_AA       ) ; // AUTOACK
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::EN_RXADDR   ) ; // RX address for all data pipe
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::SETUP_AW    ) ; // address field width
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::SETUP_RETR  ) ; // auto retransmit delay , retransmit count
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_CH       ) ; // RF channel
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_SETUP    ) ; // data rates , RF output power
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_STATUS   ) ; // interrupt for Rx and TX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P0  , 0x05 ) ; // Rx Address - pipe 0
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P1  , 0x05 ) ; // Rx Address - pipe 1
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P2  , 0x01 ) ; // Rx Address - pipe 2
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P3  , 0x01 ) ; // Rx Address - pipe 3
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P4  , 0x01 ) ; // Rx Address - pipe 4
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_ADDR_P5  , 0x01 ) ; // Rx Address - pipe 5
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::TX_ADDR     , 0x05 ) ; // Tx Address - pipe 0
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P0    ) ; // data pipe 0
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P1    ) ; // data pipe 1
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P2    ) ; // data pipe 2
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P3    ) ; // data pipe 3
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P4    ) ; // data pipe 4
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P5    ) ; // data pipe 5
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::FIFO_STATUS ) ; // RX and TX FIFO( empty )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::DYNPD       ) ; // Dynamic payload length
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::FEATURE     ) ; // Others

	if( 0 == value[ 0 ] )
	{

	}
}


void daniel::nRF24L01::ShowSpecificValue()
{

}


void daniel::nRF24L01::PowerOnOff( bool isOn )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;


	LogEvent( "nRF24L01: PowerOnOff    - [ %s ]\r\n" , ( true == isOn ) ? "true " : "false" ) ;

	bool wasCE = isCE ;
	SetCE( false ) ;

	uint8_t conf = 0x00 ;
	conf = AccessReg( TYPE::READ , REG::CONFIG ) ;
	conf = ( true == isOn ) ? ( conf | 0x02 ) : ( conf & 0xFD ) ; // PWR_UP

	AccessReg( TYPE::WRITE , REG::CONFIG , conf ) ;

	if( true == wasCE )
	{
		SetCE( true ) ;
	}
}


void daniel::nRF24L01::SetRfMode( RfMode const & mode )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	bool wasCE = isCE ;
	SetCE( false ) ;

	rfMode = ( RfMode::Unknown == mode ) ? RfMode::TX : mode ;

	uint8_t conf = 0x00 ;
	conf = AccessReg( TYPE::READ  , REG::CONFIG ) ;
	conf = ( RfMode::RX == rfMode ) ? ( conf | 0x01 ) : ( conf & 0xFE ) ; // PRIM_RX

	AccessReg( TYPE::WRITE , REG::CONFIG , conf ) ;

	if( true == wasCE || RfMode::RX == rfMode )
	{
		SetCE( true ) ;
	}
}


uint8_t daniel::nRF24L01::PushToTxFifo( uint8_t * payload , uint8_t const & length )
{
	if( nullptr == payload || 1 > length )
	{
		return 0x00 ;
	}

	if( false == dynPayload && payloadSize != length )
	{
		return 0x00 ;
	}


	namespace CMD = nordic::cmd ;

	uint8_t cmd = CMD::W_TX_PAYLOAD ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: PushToTxFifo  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	uint8_t len = ( 32 < length ) ? 32 : length ;

	spiRes = HAL_SPI_Transmit( pHandle , payload , len , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: PushToTxFifo  - [ %s ] - error - HAL_SPI_Transmit()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	SetCS( false ) ;

	LogDebug( "nRF24L01: PushToTxFifo  - " ) ;
	for( uint8_t pos = 0 ; pos < len ; ++pos )
	{
		LogDebug( "0x%02X " , payload[ pos ] ) ;
	}
	LogDebug( "\r\n" ) ;

	return 0x01 ;
}


uint8_t daniel::nRF24L01::PopFromRxFifo( uint8_t * payload , uint8_t & length )
{
	if( nullptr == payload )
	{
		return 0x00 ;
	}


	namespace CMD  = nordic::cmd  ;
	namespace TYPE = nordic::type ;

	uint8_t cmd = CMD::R_RX_PL_WID ;
	uint8_t ret = 0x00 ;
	uint8_t len = 0x00 ;

	HAL_StatusTypeDef spiRes = HAL_OK ;

	if( true == dynPayload )
	{
		SetCS( true ) ;

		spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
		if( HAL_OK != spiRes )
		{
			LogEvent( "nRF24L01: PopFromRxFifo - [ %s ] - error - HAL_SPI_TransmitReceive() - R_RX_PL_WID\r\n" , errType[ spiRes ] ) ;
			SetCS( false ) ;
			return 0x00 ;
		}

		spiRes = HAL_SPI_Receive( pHandle , & len , 1 , spiTimeOut ) ;
		if( HAL_OK != spiRes )
		{
			LogEvent( "nRF24L01: PopFromRxFifo - [ %s ] - error - HAL_SPI_Receive() - R_RX_PL_WID\r\n" , errType[ spiRes ] ) ;
			SetCS( false ) ;
			return 0x00 ;
		}

		SetCS( false ) ;

		if( 0 == len || 32 < len )
		{
			FlushFIFO( TYPE::RX ) ;
			return 0x00 ;
		}
	}
	else
	{
		len = payloadSize ;
	}

	cmd = CMD::R_RX_PAYLOAD ;
	ret = 0x00 ;

	SetCS( true ) ;

	spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: PopFromRxFifo - [ %s ] - error - HAL_SPI_TransmitReceive() - R_RX_PAYLOAD\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	spiRes = HAL_SPI_Receive( pHandle , payload , len , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: PopFromRxFifo - [ %s ] - error - HAL_SPI_Receive() - R_RX_PAYLOAD\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	SetCS( false ) ;

	length = len ;

	LogDebug( "nRF24L01: PopFromRxFifo - " ) ;
	for( uint8_t pos = 0 ; pos < length ; ++pos )
	{
		LogDebug( "0x%02X " , payload[ pos ] ) ;
	}
	LogDebug( "\r\n" ) ;

	return 0x01 ;
}


uint8_t daniel::nRF24L01::Receive( uint8_t * payload , uint8_t & length )
{
	if( nullptr == payload )
	{
		return 0 ;
	}


	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t fifoStatus = AccessReg( TYPE::READ , REG::FIFO_STATUS ) ;
	if( 0 != ( 0x01 & fifoStatus ) )
	{
		return 0 ;
	}

	uint8_t res = PopFromRxFifo( payload , length ) ;

	return res ;
}


uint8_t daniel::nRF24L01::Transmit( uint8_t * payload , uint8_t const & length )
{
	if( nullptr == payload || 1 > length )
	{
		return 0 ;
	}

	if( false == dynPayload && payloadSize != length )
	{
		return 0 ;
	}


	uint8_t len = ( 32 < length ) ? 32 : length ;

	SetCE( false ) ;

	uint8_t res = PushToTxFifo( payload , len ) ;

	SetCE( true  ) ;
	DelayUS( 20 ) ;
	SetCE( false ) ;

	return res ;
}


uint8_t daniel::nRF24L01::AccessReg( uint8_t const & accessType , uint8_t const & reg , uint8_t const & val )
{
	namespace TYPE = nordic::type ;
	namespace CMD  = nordic::cmd  ;

	//Log( "nRF24L01: AccessReg  - type [ %s ] - reg [ 0x%02X ] - val[ 0x%02X ]\r\n" , ( TYPE::READ == accessType ) ? "read " : "write" , reg , val ) ;

	uint8_t cmd = ( ( TYPE::READ == accessType ) ? CMD::R_REGISTER : CMD::W_REGISTER ) | reg ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	uint8_t res = val ;
	if( TYPE::READ == accessType )
	{
		spiRes = HAL_SPI_Receive( pHandle , & res , 1 , spiTimeOut ) ;
	}
	else
	{
		spiRes = HAL_SPI_Transmit( pHandle , & res , 1 , spiTimeOut ) ;
	}

	/**/ if( HAL_OK != spiRes && TYPE::READ == accessType )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_Receive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}
	else if( HAL_OK != spiRes && TYPE::WRITE == accessType )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_Transmit()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	LogDebug( "nRF24L01: AccessReg     - type [ %s ] - reg [ 0x%02X ] - Q val[ 0x%02X ] - R val[ 0x%02X ]\r\n" , ( TYPE::READ == accessType ) ? "read " : "write" , reg , val , res ) ;

	SetCS( false ) ;

	return res ;
}


uint8_t * daniel::nRF24L01::AccessReg( uint8_t const & accessType , uint8_t const & reg , uint8_t const & len , uint8_t const * pVal )
{
	namespace TYPE = nordic::type ;
	namespace CMD  = nordic::cmd  ;

	//Log( "nRF24L01: AccessReg  - type [ %s ] - reg [ 0x%02X ] - val[ 0x%02X ]\r\n" , ( TYPE::READ == accessType ) ? "read " : "write" , reg , val ) ;

	uint8_t cmd = ( ( TYPE::READ == accessType ) ? CMD::R_REGISTER : CMD::W_REGISTER ) | reg ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return nullptr ;
	}

	static uint8_t data[ 5 ] = { 0x00 , 0x00 , 0x00 , 0x00 , 0x00 } ;
	uint8_t dataLen = ( 5 <= len ) ? 5 : len ;

	for( uint8_t pos = 0 ; pos < dataLen ; ++pos )
	{
		data[ pos ] = pVal[ pos ] ;
	}

	if( TYPE::READ == accessType )
	{
		spiRes = HAL_SPI_Receive( pHandle , data , dataLen , spiTimeOut ) ;
	}
	else
	{
		spiRes = HAL_SPI_Transmit( pHandle , data , dataLen , spiTimeOut ) ;
	}

	/**/ if( HAL_OK != spiRes && TYPE::READ == accessType )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_Receive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return nullptr ;
	}
	else if( HAL_OK != spiRes && TYPE::WRITE == accessType )
	{
		LogEvent( "nRF24L01: AccessReg     - [ %s ] - error - HAL_SPI_Transmit()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return nullptr ;
	}

	LogDebug( "nRF24L01: AccessReg     - type [ %s ] - reg [ 0x%02X ] - Q val[ " , ( TYPE::READ == accessType ) ? "read " : "write" , reg ) ;

	for( uint8_t pos = 0 ; pos < len ; ++pos )
	{
		LogDebug( "0x%02X " , pVal[ pos ] ) ;
	}

	LogDebug( "- R val[ " ) ;

	for( uint8_t pos = 0 ; pos < dataLen ; ++pos )
	{
		LogDebug( "0x%02X " , data[ pos ] ) ;
	}

	LogDebug( "]\r\n" ) ;

	SetCS( false ) ;

	return data ;
}


void daniel::nRF24L01::FlushFIFO()
{
	namespace TYPE = nordic::type ;

	FlushFIFO( TYPE::RX ) ;
	FlushFIFO( TYPE::TX ) ;
}


void daniel::nRF24L01::FlushFIFO( uint8_t const & mode )
{
	namespace TYPE = nordic::type ;
	namespace CMD  = nordic::cmd  ;

	LogDebug( "nRF24L01: FlushFIFO     - mode [ %s ]\r\n" , ( TYPE::RX == mode ) ? "FLUSH_RX" : "FLUSH_TX" ) ;

	uint8_t cmd = ( TYPE::RX == mode ) ? CMD::FLUSH_RX : CMD::FLUSH_TX ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: FlushFIFO     - mode [ %s ] - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , ( TYPE::RX == mode ) ? "FLUSH_RX" : "FLUSH_TX" , errType[ spiRes ] ) ;
	}

	SetCS( false ) ;
}


void daniel::nRF24L01::SetUart( USART * _pUart )
{
	pUart = _pUart ;
}


void daniel::nRF24L01::LeaveLog( bool const & is )
{
	leaveLog = is ;
}


void daniel::nRF24L01::LogEvent( char const * const format , ... ) const
{
	if( false == leaveLog || nullptr == pUart )
	{
		return ;
	}

	static char szBuf[ 1024 ] ;

	va_list args ;
	va_start( args , format ) ;
	vsprintf( szBuf , format , args ) ;

	pUart->SendMessage( ( uint8_t * ) szBuf , strlen( szBuf ) ) ;

	va_end( args ) ;
}


void daniel::nRF24L01::LogDebug( char const * const format , ... ) const
{
	if( false == debugLog || nullptr == pUart )
	{
		return ;
	}

	static char szBuf[ 1024 ] ;

	va_list args ;
	va_start( args , format ) ;
	vsprintf( szBuf , format , args ) ;

	pUart->SendMessage( ( uint8_t * ) szBuf , strlen( szBuf ) ) ;

	va_end( args ) ;
}


int8_t daniel::nRF24L01::Irq()
{
	int8_t res = ( RfMode::RX == rfMode ) ? IrqRx() : IrqTx() ;

	return res ;
}


int8_t daniel::nRF24L01::IrqTx()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t status = AccessReg( TYPE::READ , REG::RF_STATUS ) ;

	uint8_t res = 0x00 ;

	/**/ if( status & 0x20 )
	{
		HAL_GPIO_TogglePin( IRQ_Port , IRQ_Pin ) ;
		status = status | 0x20 ;

		res = 1 ;
		LogDebug( "nRF24L01: IrqTx         - Transmit success ( TX_DS )\r\n" , status ) ;
	}
	else if( status & 0x10 )
	{
		status = status | 0x10 ;
		FlushFIFO( TYPE::TX ) ;

		res = 0 ;
		LogDebug( "nRF24L01: IrqTx         - Transmit failed ( MAX_RT ) - AutoACK is %s\r\n" , ( ( true == autoACK ) ? "enabled" : "disabled" ) ) ;
	}
	else
	{
		res = -1 ;
		LogEvent( "nRF24L01: IrqTx         - Transmit IRQ but unknown status 0x%02X\r\n" , status ) ;
	}

	AccessReg( TYPE::WRITE , REG::RF_STATUS , status ) ;

	return res ;
}


int8_t daniel::nRF24L01::IrqRx()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;


	uint8_t res = 0x00 ;

	while( true )
	{
		uint8_t payload[ 32 ] ;
		uint8_t length = 0 ;

		uint8_t res = Receive( payload , length ) ;
		if( 0 == res )
		{
			break ;
		}
	}

	uint8_t status = AccessReg( TYPE::READ , REG::RF_STATUS ) ;
	status = status | 0x40 ;

	AccessReg( TYPE::WRITE , REG::RF_STATUS , status ) ;

	LogDebug( "nRF24L01: IrqRx         - Receive success\r\n" , status ) ;

	return res ;
}


daniel::RfMode daniel::nRF24L01::GetRfMode() const
{
	return rfMode ;
}


void daniel::nRF24L01::DelayUS( uint16_t const & us )
{
	if( 0 == us )
	{
		return ;
	}

	uint16_t const msec = us / 1000 ;
	uint16_t const usec = us % 1000 ;

	if( 0U < msec )
	{
		HAL_Delay( msec ) ;
	}

	if( 0 == usec )
	{
		return ;
	}

	// SYSCLK --> 72MHz --> 1us --> about 9 loops
	uint32_t loops = usec * 9U ;

	__asm__ volatile (
		"1: \n\t"
		"   nop \n\t"
		"   nop \n\t"
		"   subs %0, %0, #1 \n\t"
		"   bne 1b \n\t"
		: "+r" ( loops )
		:
		: "cc", "memory"
	) ;
}
