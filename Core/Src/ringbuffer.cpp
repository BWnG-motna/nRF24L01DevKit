/*
 * ringbuffer.cpp
 *
 *  Created on: Feb 18, 2025
 *      Author: Daniel Lee
 */


#include "ringbuffer.h"
#include <new>


template < typename T >
daniel::RingBuffer< T >::RingBuffer()
	: bufferSize( 0 ) , front( 0 ) , rear( 0 ) , cnt( 0 ) , pBuf( nullptr )
{
	if( 0 != bufferSize )
	{
		pBuf = new ( std::nothrow ) T[ bufferSize ] ;
	}
}


template < typename T >
daniel::RingBuffer< T >::RingBuffer( uint16_t const & size )
	: bufferSize( size ) , front( 0 ) , rear( 0 ) , cnt( 0 ) , pBuf( nullptr )
{
	if( 0 != bufferSize )
	{
		pBuf = new ( std::nothrow ) T[ bufferSize ] ;
	}
}


template < typename T >
daniel::RingBuffer< T >::~RingBuffer()
{
	if( nullptr == pBuf )
	{
		return ;
	}

	delete [] pBuf ;
	pBuf = nullptr ;
}


template < typename T >
bool daniel::RingBuffer< T >::Alloc( uint16_t const & size )
{
	if( 0 != bufferSize || nullptr != pBuf )
	{
		return false ;
	}

	pBuf = new ( std::nothrow ) T[ size ] ;

	if( nullptr == pBuf )
	{
		return false ;
	}

	bufferSize = size ;
	front = 0 ;
	rear  = 0 ;
	cnt   = 0 ;

	return true ;
}


template < typename T >
bool daniel::RingBuffer< T >::IsEmpty() const
{
	if( nullptr == pBuf )
	{
		return true ;
	}

	return ( front == rear ) ? true : false ;
}


template < typename T >
bool daniel::RingBuffer< T >::IsFull() const
{
	if( nullptr == pBuf )
	{
		return false ;
	}

	return ( ( ( rear + 1 ) % bufferSize ) == front ) ? true : false ;
}


template < typename T >
bool daniel::RingBuffer< T >::Insert( T const & data )
{
	if( nullptr == pBuf )
	{
		return false ;
	}

	if( true == IsFull() )
	{
		return false ;
	}

	pBuf[ rear ] = data ;
	rear = ( rear + 1 ) % bufferSize ;

	++cnt ;

	return true ;
}



template < typename T >
bool daniel::RingBuffer< T >::Remove()
{
	if( nullptr == pBuf )
	{
		return false ;
	}

	if( IsEmpty() )
	{
		return false ;
	}

	front = ( front + 1 ) % bufferSize ;

	--cnt ;

	return true ;
}


template < typename T >
bool daniel::RingBuffer< T >::Remove( T * pData )
{
	if( nullptr == pBuf )
	{
		return false ;
	}

	if( IsEmpty() )
	{
		*pData = T() ;

		return false ;
	}

	*pData = pBuf[ front ] ;
	front = ( front + 1 ) % bufferSize ;

	--cnt ;

	return true ;
}


template < typename T >
uint16_t daniel::RingBuffer< T >::GetCount() const
{
	return cnt ;
}


template < typename T >
T const daniel::RingBuffer< T >::At( uint16_t const & at ) const
{
	if( nullptr == pBuf || bufferSize <= at || 0 > at )
	{
		return T() ;
	}

	uint16_t datPos = front ;

	datPos = ( datPos + at ) % bufferSize ;

	return pBuf[ datPos ] ;
}


template <  >
uint8_t const daniel::RingBuffer< uint8_t >::At( uint16_t const & at ) const
{
	if( nullptr == pBuf || bufferSize <= at || 0 > at )
	{
		return 0x00 ;
	}

	uint16_t datPos = front ;

	datPos = ( datPos + at ) % bufferSize ;

	return pBuf[ datPos ] ;
}


template < typename T >
T const & daniel::RingBuffer< T >::operator[]( uint16_t const & idx ) const
{
	static T tmp ;

	if( nullptr == pBuf || bufferSize <= idx )
	{
		return tmp ;
	}

	uint16_t datPos = front ;

	datPos = ( datPos + idx ) % bufferSize ;

	return pBuf[ datPos ] ;
}


template < >
uint8_t const & daniel::RingBuffer< uint8_t >::operator[]( uint16_t const & idx ) const
{
	static uint8_t tmp = 0x00 ;

	if( nullptr == pBuf || bufferSize <= idx )
	{
		return tmp ;
	}

	uint16_t datPos = front ;

	datPos = ( datPos + idx ) % bufferSize ;

	return pBuf[ datPos ] ;
}


template < typename T >
void daniel::RingBuffer< T >::Reset()
{
	front = 0 ;
	rear  = 0 ;
	cnt   = 0 ;
}

template < typename T >
uint16_t daniel::RingBuffer< T >::GetSize() const
{
	return bufferSize ;
}


template < typename T >
void daniel::RingBuffer< T >::RemoveUntil( T const & dat )
{
	while( ( pBuf[ front ] != dat ) && ( false == IsEmpty() ) )
	{
		front = ( front + 1 ) % bufferSize ;
		--cnt ;
	}
}

template class daniel::RingBuffer< uint8_t > ;
