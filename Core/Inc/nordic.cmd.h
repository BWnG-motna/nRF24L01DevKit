#pragma once

namespace daniel
{

namespace nordic
{

namespace cmd
{

/* SPI commands */
constexpr uint8_t const R_REGISTER         = 0b00000000 ; // read  command/status register
constexpr uint8_t const W_REGISTER         = 0b00100000 ; // write command/status register
constexpr uint8_t const R_RX_PAYLOAD       = 0b01100001 ; // read  RX payload ( in RX mode )
constexpr uint8_t const W_TX_PAYLOAD       = 0b10100000 ; // write TX payload
constexpr uint8_t const FLUSH_TX           = 0b11100001 ; // flush TX_FIFO in TX mode
constexpr uint8_t const FLUSH_RX           = 0b11100010 ; // flush RX_FIFO in RX mode - should not be executed during transmission of acknowledge
constexpr uint8_t const REUSE_TX_PL        = 0b11100011 ; // for PTX device
constexpr uint8_t const R_RX_PL_WID        = 0b01100000 ; //
constexpr uint8_t const W_ACK_PAYLOAD      = 0b10101000 ; // in RX mode, transmit data with ACK
constexpr uint8_t const W_TX_PAYLOAD_NOACK = 0b10110000 ; // in TX mode, disable AUTOACK on this specific packet
constexpr uint8_t const NOP                = 0b11111111 ; // no operation - might be used to read the STATUS register

} // namespace cmd

} // namespace nordic

} // namespace daniel
