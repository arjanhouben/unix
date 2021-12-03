#pragma once

#include "arjan/unix/file.hpp"

namespace arjan {
namespace unix {

struct pipe : std::array< unix::file, 2 >
{
	enum direction
	{
		input = 0,
		output = 1
	};

	inline explicit operator bool() const noexcept
	{
		return operator[]( input ) && operator[]( output );
	}
	
	inline auto& open() noexcept
	{
		int descriptors[ 2 ] = { -1 };
		::pipe( descriptors );
		operator[]( input ).reset( descriptors[ input ] );
		operator[]( output ).reset( descriptors[ output ] );
		return *this;
	}

	inline void close() noexcept
	{
		operator[]( input ).reset();
		operator[]( output ).reset();
	}
};

}}