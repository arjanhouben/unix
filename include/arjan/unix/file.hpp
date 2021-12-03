#pragma once

#include <compare>

namespace arjan {
namespace unix {

struct file
{
	enum class mode 
	{
		read = O_RDONLY,
		write = O_WRONLY,
		read_write = O_RDWR
	};

	inline explicit file( const char *path, mode m ) :
		no_( open( path, static_cast< std::underlying_type_t< mode > >( m ) ) ) {}

	inline explicit file( int no = -1 ) noexcept :
		no_( no ) {}
	
	inline file( file &&rhs ) noexcept :
		no_( rhs.release() ) {}
	
	inline file& operator = ( file &&rhs ) noexcept
	{
		reset( rhs.release() );
		return *this;
	}
	
	inline ~file() noexcept
	{
		if ( no_ >= 0 )
		{
			::close( no_ );
		}
	}

	auto operator <=> ( const file &rhs ) const = default;

	inline explicit operator bool() const noexcept
	{
		return no_ >= 0;
	}
	
	inline int get() const noexcept
	{
		return no_;
	}
	
	inline int release() noexcept
	{
		int r = -1;
		std::swap( no_, r );
		return r;
	}
	
	inline void reset( int no = -1 ) noexcept
	{
		std::swap( no_, no );
		if ( no >= 0 )
		{
			::close( no );
		}
	}
	
	private:
		int no_;
};

}}
