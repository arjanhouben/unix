#pragma once

#include <streambuf>

#include "arjan/posix/file.hpp"

namespace arjan {
namespace posix {

template < typename char_type = char >
struct basic_streambuf : std::basic_streambuf< char_type >
{
	using base = std::basic_streambuf< char_type >;
	using int_type = typename base::int_type;
	using traits_type = typename base::traits_type;
	
	explicit basic_streambuf( file f ) noexcept  :
		base(),
		file_( std::move( f ) ),
		gcurrent_( traits_type::eof() ) {}

	auto reset( posix::file &&fileno = file() ) noexcept
	{
		std::swap( file_, fileno );
		gcurrent_ = traits_type::eof();
		return std::move( fileno );
	}
	
	std::streamsize xsgetn( char_type* dst, std::streamsize count ) override
	{
		const auto n = ::read( file_.get(), dst, static_cast< size_t >( count ) );
		if ( n == -1 )
		{
			return 0;
		}
		return n;
	}

	int_type underflow() override
	{
		if ( gcurrent_ == traits_type::eof() )
		{
			return uflow();
		}
		return gcurrent_;
	}
	
	int_type uflow() override
	{
		char_type c;
		if ( xsgetn( &c, 1 ) )
		{
			gcurrent_ = traits_type::to_int_type( c );
		}
		else
		{
			gcurrent_ = traits_type::eof();
		}
		return gcurrent_;
	}

	std::streamsize xsputn( const char_type* src, std::streamsize count ) override
	{
		const auto n = ::write( file_.get(), src, static_cast< size_t >( count ) );
		if ( n == -1 )
		{
			return 0;
		}
		return n;
	}

	int_type overflow( int_type inp = traits_type::eof() ) override
	{
		auto c = traits_type::to_char_type( inp );
		if ( xsputn( &c, 1 ) )
		{
			return traits_type::to_int_type( c );
		}
		return traits_type::eof();
	}
	
	private:
	
		file file_;
		int_type gcurrent_;
};

using streambuf = basic_streambuf< char >;

}}