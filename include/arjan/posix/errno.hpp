#pragma once

#include <exception>

namespace arjan {
namespace posix {

template < typename F, typename ...Args >
auto check_errno( F &&f, Args &&...args )
{
	const auto result = f( std::forward< Args >( args )... );
	if ( result == -1 )
	{
		throw std::system_error( errno, std::system_category() );
	}
	return result;
}

}}