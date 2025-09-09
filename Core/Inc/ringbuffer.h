/*
 * ringbuffer.h
 *
 *  Created on: Feb 18, 2025
 *      Author: Daniel Lee
 */

#pragma once

#include "common.h"


namespace daniel
{

template < typename T >
class RingBuffer final
{
private :
	uint16_t bufferSize ;

private :
	uint16_t front ;
	uint16_t rear  ;

	uint16_t cnt  ;
	T * pBuf ;


public :
	bool IsEmpty() const ;
	bool IsFull() const ;
	bool Insert( T const & data ) ;
	bool Remove() ;
	bool Remove( T * pData ) ;
	void Reset() ;

	void Stripping( uint8_t * pBuf ) ;

	uint16_t GetCount() const ;

	bool Alloc( uint16_t const & size ) ;

	T const At( uint16_t const & at ) const ;
	T const & operator[]( uint16_t const & idx ) const ;

	uint16_t GetSize() const ;

	void RemoveUntil( T const & dat ) ;

public:
	 RingBuffer() ;
	 RingBuffer( uint16_t const & size ) ;
	~RingBuffer() ;
} ;

} // namespace daniel


