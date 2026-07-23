#pragma once

#include "common.h"
#include "usart.h"

#include "RfARD.h"
#include "RfMode.h"
#include "RfPower.h"
#include "RfLnaGain.h"
#include "RfDataRate.h"

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
	bool isCE ;

private :
	USART * pUart ;

private :
	uint8_t payloadSize ;

private :
	static    char const * errType [ 4 ] ;
	static uint8_t const   rfAddrP0[ 5 ] ;
	static uint8_t const   rfAddrP1[ 5 ] ;
	static uint8_t const   rfAddrP2 ;
	static uint8_t const   rfAddrP3 ;
	static uint8_t const   rfAddrP4 ;
	static GPIO_TypeDef  * IRQ_Port ;

public :
	static constexpr uint16_t IRQ_Pin  = GPIO_PIN_0 ;

private :
	bool leaveLog ;
	bool debugLog ;
	bool autoACK  ;

private :
	uint8_t    rfChannel ;

private :
	RfARD      rfARD ;
	RfMode     rfMode ;
	RfPower    rfPower ;
	RfLnaGain  rfLnaGain ;
	RfDataRate rfDataRate ;

private :
	uint8_t    rfARC ;

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

	uint8_t GetSetupRetr()  const ;
	uint8_t GetRfSetupVal() const ;
	uint8_t PushToTxFifo ( uint8_t * payload ) ;
	uint8_t PopFromRxFifo( uint8_t * payload ) ;

private :
	void Inspection() ;

private :
	void LogEvent( char const * const format , ... ) const ;
	void LogDebug( char const * const format , ... ) const ;

private :
	void DelayUS( uint16_t const & us ) ;

private :
	int8_t IrqTx() ;
	int8_t IrqRx() ;

public :
	int8_t Irq() ;

public :
	void SetUart( USART * _pUart ) ;
	void LeaveLog( bool const & is ) ;

public :
	void SetARD( RfARD   const & ard ) ;
	void SetARC( uint8_t const & arc ) ;

public :
	uint8_t Receive ( uint8_t * payload ) ;
	uint8_t Transmit( uint8_t * payload ) ;

public :
	void ShowSpecificValue() ;
	RfMode GetRfMode() const ;

public :
	void Begin( RfMode const & mode ) ;
	void End() ;

public :
	void SetChannel( uint8_t const & ch ) ;

public :
	nRF24L01( SPI_HandleTypeDef * pHandle , bool const & leaveLog = false ) ;
	nRF24L01( SPI_HandleTypeDef * pHandle , USART * pUart , bool const & leaveLog = false ) ;

} ; // class

} // namespace daniel
