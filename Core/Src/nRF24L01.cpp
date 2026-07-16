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


daniel::nRF24L01::nRF24L01( SPI_HandleTypeDef * _pHandle , bool const & _leaveLog )
	: pHandle( _pHandle )   , isCS( false )     , pUart( nullptr ) , payloadSize( 32 ) , rfMode( RfMode::Unknown ) ,
	  leaveLog( _leaveLog ) , debugLog( false ) , autoACK( true )
{
	SetCS( false ) ;
	LogEvent( "nRF24L01: created\r\n" ) ;
}


daniel::nRF24L01::nRF24L01( SPI_HandleTypeDef * _pHandle , USART * _pUart , bool const & _leaveLog )
	: pHandle( _pHandle )   , isCS( false )     , pUart( _pUart ) , payloadSize( 32 ) , rfMode( RfMode::Unknown ) ,
	  leaveLog( _leaveLog ) , debugLog( false ) , autoACK( true )
{
	SetCS( false ) ;
	LogEvent( "nRF24L01: created\r\n" ) ;
}


void daniel::nRF24L01::SetCS( bool const & isEnable )
{
	// CSN signal
	// RX mode - CS must be always enabled
	// TX mode - CS must be enabled during sending data, otherwise, must be disabled
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_5 , ( true == isEnable ) ? GPIO_PIN_RESET : GPIO_PIN_SET ) ;
}


void daniel::nRF24L01::SetCE( bool const & isEnable )
{
	// CE signal
	// CE must be enabled during SPI communication
	HAL_GPIO_WritePin( GPIOC , GPIO_PIN_4 , ( true == isEnable ) ? GPIO_PIN_SET : GPIO_PIN_RESET ) ;
}


void daniel::nRF24L01::Begin( RfMode const & mode )
{
	LogEvent( "nRF24L01: Begin\r\n" ) ;

	rfMode = mode ;

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


void daniel::nRF24L01::Init()
{
	LogEvent( "nRF24L01: Init\r\n" ) ;
	SetCS( true ) ;
	SetCE( true ) ;

	namespace TYPE = nordic::type ;
	namespace REG  = nordic::reg  ;

	uint8_t enAA = ( true == autoACK ) ? 0x01 : 0x00 ;

	AccessReg( TYPE::WRITE , REG::CONFIG      , 0x08 ) ; // enable CRC
	AccessReg( TYPE::WRITE , REG::EN_AA       , enAA ) ; // enable AUTOACK for P0
	AccessReg( TYPE::WRITE , REG::EN_RXADDR   , 0x3F ) ; // enable RX address for all data pipe
	AccessReg( TYPE::WRITE , REG::SETUP_AW    , 0x03 ) ; // address field width( 5 )
	AccessReg( TYPE::WRITE , REG::SETUP_RETR  , 0x03 ) ; // auto retransmit delay( 250us ) , retransmit count( 3 )
	AccessReg( TYPE::WRITE , REG::RF_CH       , 0x4C ) ; // RF channel ( 76 )
	AccessReg( TYPE::WRITE , REG::RF_SETUP    , 0x06 ) ; // data rates ( 1Mbps ) , RF output power( 0 dBm )
	AccessReg( TYPE::WRITE , REG::RF_STATUS   , 0x7F ) ; // interrupt enabled for Rx and TX , RX FIFO empty, TX FIFO ready
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P0  , 0x05 ,   rfAddrP0 ) ;
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P1  , 0x05 ,   rfAddrP1 ) ;
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P1  , 0x01 , & rfAddrP2 ) ;
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P2  , 0x01 , & rfAddrP3 ) ;
	AccessReg( TYPE::WRITE , REG::RX_ADDR_P3  , 0x01 , & rfAddrP4 ) ;
	AccessReg( TYPE::WRITE , REG::TX_ADDR     , 0x05 ,   rfAddrP0 ) ;
	AccessReg( TYPE::WRITE , REG::RX_PW_P0    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P1    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P2    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P3    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P4    , payloadSize ) ; // payload size
	AccessReg( TYPE::WRITE , REG::RX_PW_P5    , payloadSize ) ; // payload size
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
	value[ valuePos++ ] = AccessReg( TYPE::READ , REG::RF_SETUP    ) ; // data rates ( 1Mbps ) , RF output power( 0 dBm )
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

	if( 0 == value[ 0 ] )
	{

	}
}


void daniel::nRF24L01::ShowSpecificValue()
{

}


void daniel::nRF24L01::PowerOnOff( bool isOn )
{
	LogEvent( "nRF24L01: PowerOnOff    - [ %s ]\r\n" , ( true == isOn ) ? "true " : "false" ) ;

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
		LogEvent( "nRF24L01: PushToTxFifo  - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	spiRes = HAL_SPI_Transmit( pHandle , payload , payloadSize , spiTimeOut ) ;
	if( HAL_OK != spiRes )
	{
		LogEvent( "nRF24L01: PushToTxFifo  - [ %s ] - error - HAL_SPI_Transmit()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	SetCS( false ) ;

	LogDebug( "nRF24L01: PushToTxFifo  - " ) ;
	for( uint8_t pos = 0 ; pos < payloadSize ; ++pos )
	{
		LogDebug( "0x%02X " , payload[ pos ] ) ;
	}
	LogDebug( "\r\n" ) ;

	return 0x01 ;
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
		LogEvent( "nRF24L01: PopFromRxFifo - [ %s ] - error - HAL_SPI_TransmitReceive()\r\n" , errType[ spiRes ] ) ;
		SetCS( false ) ;
		return 0x00 ;
	}

	spiRes = HAL_SPI_Receive( pHandle , payload , payloadSize , spiTimeOut ) ;

	SetCS( false ) ;

	LogDebug( "nRF24L01: PopFromRxFifo - " ) ;
	for( uint8_t pos = 0 ; pos < payloadSize ; ++pos )
	{
		LogDebug( "0x%02X " , payload[ pos ] ) ;
	}
	LogDebug( "\r\n" ) ;

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
	SetCE( false ) ;

	uint8_t res = PushToTxFifo( payload ) ;

	SetCE( true  ) ;
	DelayUS( 20 ) ;
	SetCE( false ) ;

	if( 0 != res )
	{

	}
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
	int8_t res = ( RfMode::TX == rfMode ) ? IrqTx() : IrqRx() ;

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

	Receive( recvData ) ;

	uint8_t status = AccessReg( TYPE::READ , REG::RF_STATUS ) ;
	status = status | 0x40 ;

	AccessReg( TYPE::WRITE , REG::RF_STATUS , status ) ;

	FlushFIFO( TYPE::RX ) ;

	LogDebug( "nRF24L01: IrqRx         - Receive success\r\n" , status ) ;

	return 1 ;
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
