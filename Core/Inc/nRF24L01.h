#pragma once

#include "common.h"
#include "usart.h"
#include "RfMode.h"

#include "stm32f1xx_hal_spi.h"
#include "stm32f103xe.h"


namespace daniel
{

class nRF24L01
{

private :
	static constexpr uint16_t spiTimeOut = 1000 ;

private :
	SPI_HandleTypeDef * pHandle ;

private :
	bool isCS ;

private :
	USART * pUart ;

private :
	uint8_t payloadSize ;
	uint8_t recvData[ 32 ] ;

private :
	RfMode rfMode ;

private :
	static    char const * errType[ 4 ] ;
	static uint8_t const   rfAddr [ 5 ] ;
	static GPIO_TypeDef  * IRQ_Port ;

public :
	static constexpr uint16_t IRQ_Pin  = GPIO_PIN_0 ;

private :
	bool leaveLog ;

private :
	void Init() ;
	void SetCS( bool const & isEnable = true  ) ;
	void SetCE( bool const & isEnable = false ) ;

private :
	uint8_t   AccessReg( uint8_t const & accessType , uint8_t const & reg , uint8_t const & val = 0x00 ) ;
	uint8_t * AccessReg( uint8_t const & accessType , uint8_t const & reg , uint8_t const & len , uint8_t const * pVal ) ;
	uint8_t GetStatus() ;

private :
	void PowerOnOff( bool isOn ) ;

	void FlushFIFO() ;
	void FlushFIFO( uint8_t const & type ) ;

	void SetRfMode( RfMode const & mode ) ;

	uint8_t PushToTxFifo ( uint8_t * payload ) ;
	uint8_t PopFromRxFifo( uint8_t * payload ) ;

private :
	void Inspection() ;

private :
	void IrqTx() ;
	void IrqRx() ;

private :
	void Log( char const * const format , ... ) const ;

public :
	void SetUart( USART * _pUart ) ;

public :
	void Irq() ;
	void Receive ( uint8_t * payload ) ;
	void Transmit( uint8_t * payload ) ;
	void ShowSpecificValue() ;

public :
	void Begin( RfMode const & mode ) ;
	void End() ;

public :
	nRF24L01( SPI_HandleTypeDef * pHandle ) ;

} ; // class

} // namespace daniel
