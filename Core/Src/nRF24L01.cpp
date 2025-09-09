#include "nRF24L01.h"

#include "nordic.cmd.h"
#include "nordic.reg.h"
#include "nordic.type.h"

#include <stdio.h>  // vsprintf()
#include <stdarg.h> // va_list , va_start() , va_end()
#include <string.h> // for strlen()


GPIO_TypeDef * daniel::nRF24L01::IRQ_Port = reinterpret_cast< GPIO_TypeDef * >( GPIOB_BASE ) ;


daniel::nRF24L01::nRF24L01( SPI_HandleTypeDef * _pHandle )
	: pHandle( _pHandle ) , isCS( false ) , pUart( nullptr ) , payloadSize( 32 ) , rfMode( RfMode::Unknown )
{
	SetCS( false ) ;
	Log( "nRF24L01: created\r\n" ) ;
}


void daniel::nRF24L01::SetCS( bool const & isEnable )
{
	// CSN signal
	//Log( "nRF24L01: SetCS      - [ %s ]\r\n" , ( true == isEnable ) ? "true " : "false" ) ;
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_5 , ( true == isEnable ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::nRF24L01::SetCE( bool const & isEnable )
{
	// CE signal
	Log( "nRF24L01: SetCE      - [ %s ]\r\n" , ( true == isEnable ) ? "true " : "false" ) ;
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_4 , ( true == isEnable ) ? GPIO_PIN_SET : GPIO_PIN_RESET ) ;
}


void daniel::nRF24L01::Begin( RfMode const & mode )
{
	Log( "nRF24L01: Begin\r\n" ) ;

	rfMode = mode ;

	Init() ;
	SetRfMode( mode ) ;
	PowerOnOff( nordic::type::ON ) ;
	Inspection() ;
}


void daniel::nRF24L01::End()
{
	Log( "nRF24L01: End\r\n" ) ;

	PowerOnOff( nordic::type::OFF ) ;
}


void daniel::nRF24L01::Init()
{
	Log( "nRF24L01: Init\r\n" ) ;
	SetCS( true ) ;
	SetCE( true ) ;

	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	AccessReg( TYPE::WRITE , REG::CONFIG      , 0x08 ) ; // enable CRC
	AccessReg( TYPE::WRITE , REG::EN_AA       , 0x3F ) ; // enable AUTOACK for all data pipe
	AccessReg( TYPE::WRITE , REG::EN_RXADDR   , 0x3F ) ; // enable RX address for all data pipe
	AccessReg( TYPE::WRITE , REG::SETUP_AW    , 0x03 ) ; // address field width( 5 )
	AccessReg( TYPE::WRITE , REG::SETUP_RETR  , 0x03 ) ; // auto retransmit delay( 250us ) , retransmit count( 3 )
	AccessReg( TYPE::WRITE , REG::RF_CH       , 0x00 ) ; // RF channel ( 0 )
	AccessReg( TYPE::WRITE , REG::RF_SETUP    , 0x0E ) ; // data rates ( 2Mbps ) , RF output power( 0 dBm )
	AccessReg( TYPE::WRITE , REG::RF_STATUS   , 0x7F ) ; // interrupt enabled for Rx and TX , RX FIFO empty, TX FIFO ready
	AccessReg( TYPE::WRITE , REG::RX_PW_P0    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P1    , 0x00 ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P2    , 0x00 ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P3    , 0x00 ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P4    , 0x00 ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P5    , 0x00 ) ;
	AccessReg( TYPE::WRITE , REG::FIFO_STATUS , 0x11 ) ; // RX and TX FIFO( empty )
	AccessReg( TYPE::WRITE , REG::DYNPD       , 0x00 ) ; // No Dynamic payload length
	AccessReg( TYPE::WRITE , REG::FEATURE     , 0x00 ) ; // Others

	FlushFIFO() ;

	SetCE( false ) ;
}

void daniel::nRF24L01::Inspection()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t value[ 17 ] ;
	uint8_t valuePos = 0 ;

	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::CONFIG      ) ; // enable CRC
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::EN_AA       ) ; // enable AUTOACK for all data pipe
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::EN_RXADDR   ) ; // enable RX address for all data pipe
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::SETUP_AW    ) ; // address field width( 5 )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::SETUP_RETR  ) ; // auto retransmit delay( 250us ) , retransmit count( 3 )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_CH       ) ; // RF channel ( 0 )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_SETUP    ) ; // data rates ( 2Mbps ) , RF output power( 0 dBm )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_STATUS   ) ; // interrupt enabled for Rx and TX , RX FIFO empty, TX FIFO ready
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P0    ) ; // data pipe 0 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P1    ) ; // data pipe 1 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P2    ) ; // data pipe 2 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P3    ) ; // data pipe 3 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P4    ) ; // data pipe 4 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RX_PW_P5    ) ; // data pipe 5 not used in RX
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::FIFO_STATUS ) ; // RX and TX FIFO( empty )
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::DYNPD       ) ; // No Dynamic payload length
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::FEATURE     ) ; // Others

	for( uint8_t pos = 0 ; pos < valuePos ; ++pos )
	{
		Log( "nRF24L01: Insp - 0x%02X\r\n" , value[ pos ] ) ;
	}
}


void daniel::nRF24L01::PowerOnOff( bool isOn )
{
	Log( "nRF24L01: PowerOnOff - [ %s ]\r\n" , ( true == isOn ) ? "true " : "false" ) ;
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t conf = AccessReg( TYPE::READ , REG::CONFIG ) ;

	conf = ( true == isOn ) ? ( conf | 0x02 ) : ( conf & 0xFD ) ;

	AccessReg( TYPE::WRITE , REG::CONFIG , conf ) ;
}


void daniel::nRF24L01::SetRfMode( RfMode const & mode )
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t val = AccessReg( TYPE::READ  , REG::CONFIG ) ;

	uint8_t confVal = ( RfMode::RX == mode ) ? ( val | 0x01 ) : ( val & 0xFE ) ;

	AccessReg( TYPE::WRITE , REG::CONFIG , confVal ) ;
}


uint8_t daniel::nRF24L01::PushToTxFifo( uint8_t * payload )
{
	namespace CMD = nordic::cmd ;

	uint8_t cmd = CMD::W_TX_PAYLOAD ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		Log( "nRF24L01: AccessReg  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
	}

	spiRes = HAL_SPI_Transmit( pHandle , payload , payloadSize , spiTimeOut ) ;

	SetCS( false ) ;

	return ret ;
}


uint8_t daniel::nRF24L01::PopFromRxFifo( uint8_t * payload )
{
	namespace CMD = nordic::cmd ;

	uint8_t cmd = CMD::R_RX_PAYLOAD ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		Log( "nRF24L01: AccessReg  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
	}

	spiRes = HAL_SPI_Receive( pHandle , payload , payloadSize , spiTimeOut ) ;

	SetCS( false ) ;

	return ret ;
}


void daniel::nRF24L01::Receive ( uint8_t * payload )
{
	uint8_t res = PopFromRxFifo( payload ) ;

	if( 0 != res )
	{

	}

	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t val = AccessReg( TYPE::READ , REG::RF_STATUS ) ;
	uint8_t confVal = val | 0x40 ;

	val = AccessReg( TYPE::WRITE , REG::RF_STATUS , confVal ) ;
}


void daniel::nRF24L01::Transmit( uint8_t * payload )
{
	uint8_t res = PushToTxFifo( payload ) ;

	if( 0 != res )
	{

	}

}


uint8_t daniel::nRF24L01::AccessReg( uint8_t const & accessType , uint8_t const & reg , uint8_t const & val )
{
	namespace TYPE = nordic::type ;
	namespace CMD  = nordic::cmd  ;

	Log( "nRF24L01: AccessReg  - type [ %s ] - reg [ 0x%02X ] - val[ 0x%02X ]\r\n" , ( TYPE::READ == accessType ) ? "read " : "write" , reg , val ) ;

	uint8_t cmd = ( ( TYPE::READ == accessType ) ? CMD::R_REGISTER : CMD::W_REGISTER ) | reg ;
	uint8_t ret = 0x00 ;

	SetCS( true ) ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		Log( "nRF24L01: AccessReg  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
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

	SetCS( false ) ;

	if( HAL_OK != spiRes && TYPE::READ == accessType )
	{
		Log( "nRF24L01: AccessReg  - [ %s ] - error - HAL_SPI_Receive()\r\n" , errType[ spiRes ] ) ;
	}
	else if( HAL_OK != spiRes && TYPE::WRITE == accessType )
	{
		Log( "nRF24L01: AccessReg  - [ %s ] - error - HAL_SPI_Transmit()\r\n" , errType[ spiRes ] ) ;
	}

	return res ;
}


void daniel::nRF24L01::FlushFIFO() const
{
	namespace TYPE = nordic::type ;

	FlushFIFO( TYPE::RX ) ;
	FlushFIFO( TYPE::TX ) ;
}


void daniel::nRF24L01::FlushFIFO( uint8_t const & mode ) const
{
	namespace TYPE = nordic::type ;
	namespace CMD  = nordic::cmd  ;

	Log( "nRF24L01: FlushFIFO  - mode [ %s ]\r\n" , ( TYPE::RX == mode ) ? "FLUSH_RX" : "FLUSH_TX" ) ;

	uint8_t cmd = ( TYPE::RX == mode ) ? CMD::FLUSH_RX : CMD::FLUSH_TX ;
	uint8_t ret = 0x00 ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle , & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		Log( "nRF24L01: FlushFIFO  - mode [ %s ] - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , ( TYPE::RX == mode ) ? "FLUSH_RX" : "FLUSH_TX" , errType[ spiRes ] ) ;
	}
}


uint8_t daniel::nRF24L01::GetStatus()
{
	namespace CMD  = nordic::cmd  ;

	SetCS( true ) ;

	uint8_t cmd = CMD::NOP ;
	uint8_t ret = 0x00 ;

	HAL_StatusTypeDef spiRes = HAL_SPI_TransmitReceive( pHandle ,  & cmd , & ret , 1 , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		Log( "nRF24L01: GetStatus  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
	}

	SetCS( false ) ;

	return ret ;
}


void daniel::nRF24L01::SetUart( USART * _pUart )
{
	pUart = _pUart ;
}


void daniel::nRF24L01::Log( char const * const format , ... ) const
{
	if( false == leaveLog )
	{
		return ;
	}

	if( nullptr == pUart )
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


void daniel::nRF24L01::Irq()
{
	( RfMode::TX == rfMode ) ? IrqTx() : IrqRx() ;
}


void daniel::nRF24L01::IrqTx()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t status = AccessReg( TYPE::READ , REG::RF_STATUS ) ;

	if( status & 0x20 )
	{
		HAL_GPIO_TogglePin( IRQ_Port , IRQ_Pin ) ;
		status = status | 0x20 ;
	}
	else
	{
		HAL_GPIO_WritePin( IRQ_Port , IRQ_Pin , GPIO_PIN_SET ) ;
		status = status | 0x10 ;
	}

	AccessReg( TYPE::WRITE , REG::RF_STATUS , status ) ;
}


void daniel::nRF24L01::IrqRx()
{
	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	Receive( recvData ) ;

	uint8_t status = AccessReg( TYPE::READ , REG::RF_STATUS ) ;
	status = status | 0x40 ;

	AccessReg( TYPE::WRITE , REG::RF_STATUS , status ) ;
}
