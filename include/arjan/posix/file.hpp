#pragma once

#include <compare>
#include <fcntl.h>
#include <unistd.h>

namespace arjan {
namespace posix {

struct file
{
	static constexpr int invalid = -1;

	enum class mode 
	{
		read = O_RDONLY,
		write = O_WRONLY,
		read_write = O_RDWR
	};

	inline explicit file( const char *path, mode m ) :
		no_( open( path, static_cast< std::underlying_type_t< mode > >( m ) ) ) {}

	inline explicit file( const std::string &path, mode m ) :
		file( path.c_str(), m ) {}
	
	constexpr inline explicit file( int no = invalid ) noexcept :
		no_( no ) {}
	
	constexpr inline file( file &&rhs ) noexcept :
		no_( rhs.release() ) {}
	
	constexpr inline file& operator = ( file &&rhs ) noexcept
	{
		reset( rhs.release() );
		return *this;
	}
	
	constexpr inline ~file() noexcept
	{
		reset();
	}

	constexpr auto operator <=> ( const file &rhs ) const noexcept = default;

	constexpr inline bool valid() const noexcept
	{
		return no_ != invalid;
	}

	constexpr inline explicit operator bool() const noexcept
	{
		return valid();
	}
	
	constexpr inline int get() const noexcept
	{
		return no_;
	}
	
	constexpr inline int release() noexcept
	{
		int r = invalid;
		std::swap( no_, r );
		return r;
	}
	
	constexpr inline void reset( int no = invalid ) noexcept
	{
		std::swap( no_, no );
		if ( valid() )
		{
			::close( no );
		}
	}
	
	private:

		int no_;
};

}}
