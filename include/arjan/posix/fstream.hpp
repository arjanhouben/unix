#pragma once

#include "arjan/posix/streambuf.hpp"

namespace arjan {
namespace posix {

template < typename stream_base >
struct basic_fstream : stream_base
{
	using streambuf_type = basic_streambuf< typename stream_base::char_type >;

	basic_fstream( file f ) :
		basic_fstream( streambuf_type( std::move( f ) ) ) {}

	basic_fstream( streambuf_type buf ) :
		stream_base( &buffer_ ),
		buffer_( std::move( buf ) ) {}
	
	private:
		streambuf_type buffer_;
};

typedef basic_fstream< std::istream > ifstream;
typedef basic_fstream< std::ostream > ofstream;

}}