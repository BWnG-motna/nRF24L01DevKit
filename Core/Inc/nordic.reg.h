#pragma once

namespace daniel
{

namespace nordic
{

namespace reg
{

/* SPI registers */
constexpr uint8_t const CONFIG      = 0x00 ;
constexpr uint8_t const EN_AA       = 0x01 ; // enhanced shockburst
constexpr uint8_t const EN_RXADDR   = 0x02 ;
constexpr uint8_t const SETUP_AW    = 0x03 ;
constexpr uint8_t const SETUP_RETR  = 0x04 ;
constexpr uint8_t const RF_CH       = 0x05 ;
constexpr uint8_t const RF_SETUP    = 0x06 ;
constexpr uint8_t const RF_STATUS   = 0x07 ;
constexpr uint8_t const OBSERVE_TX  = 0x08 ;
constexpr uint8_t const RPD         = 0x09 ;
constexpr uint8_t const RX_ADDR_P0  = 0x0A ;
constexpr uint8_t const RX_ADDR_P1  = 0x0B ;
constexpr uint8_t const RX_ADDR_P2  = 0x0C ;
constexpr uint8_t const RX_ADDR_P3  = 0x0D ;
constexpr uint8_t const RX_ADDR_P4  = 0x0E ;
constexpr uint8_t const RX_ADDR_P5  = 0x0F ;
constexpr uint8_t const TX_ADDR     = 0x10 ;
constexpr uint8_t const RX_PW_P0    = 0x11 ;
constexpr uint8_t const RX_PW_P1    = 0x12 ;
constexpr uint8_t const RX_PW_P2    = 0x13 ;
constexpr uint8_t const RX_PW_P3    = 0x14 ;
constexpr uint8_t const RX_PW_P4    = 0x15 ;
constexpr uint8_t const RX_PW_P5    = 0x16 ;
constexpr uint8_t const FIFO_STATUS = 0x17 ;
constexpr uint8_t const DYNPD       = 0x1C ;
constexpr uint8_t const FEATURE     = 0x1D ;

} // namespace reg

} // namespace nordic

} // namespace daniel
