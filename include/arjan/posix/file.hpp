#pragma once

#include <compare>
#include <fcntl.h>
#include <unistd.h>

#include "arjan/posix/errno.hpp"

namespace arjan {
namespace posix {

struct file
{	struct mode 
	{
		static constexpr auto read = O_RDONLY;
		static constexpr auto write = O_WRONLY;
		static constexpr auto read_write = O_RDWR;
		static constexpr auto create = O_CREAT;
		static constexpr auto truncate = O_TRUNC;
	};
	
	static constexpr int invalid = -1;
	
	inline explicit file( const std::string &path, int m ) :
		file( path.c_str(), m ) {}

	inline explicit file( const char *path, int m ) :
		no_( check_errno( open, path, m ) ) {}
	
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
