#pragma once

#include <future>
#include <utility>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#ifdef __linux__
#include <wait.h>
#endif

#include "arjan/unix_tools/file.hpp"
#include "arjan/unix_tools/pipe.hpp"

namespace arjan {
namespace unix_tools {
namespace process {

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

struct result_value
{
	inline result_value( int expected, std::future< int > &&f ) :
		expected_( expected ),
		future_( std::move( f ) ),
		value_( std::numeric_limits< int >::max() ) {}
	
	inline ~result_value() noexcept( false )
	{
		if ( future_.valid() )
		{
			if ( exception_count == std::uncaught_exceptions() )
			{
				future_.get();
			}
		}
	}

	inline explicit operator bool() 
	{
		return expected_ == value();
	}
	
	inline bool finished() const noexcept
	{
		if ( future_.valid() )
		{
			return future_.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
		}
		return true;
	}
	
	inline int expected() const noexcept
	{
		return expected_;
	}
	
	inline int value()
	{
		if ( future_.valid() )
		{
			value_ = future_.get();
		}
		return value_;
	}
	
	private:
	
		const int expected_;
		std::future< int > future_;
		int value_;
		const int exception_count = std::uncaught_exceptions();
};

struct handle
{
	explicit operator bool()
	{
		return static_cast< bool >( result );
	}

	const int pid;
	result_value result;
	unix_tools::file cin, cout, cerr;
};

enum class redirects
{
	parent,
	pipe,
	null
};

struct options : std::array< redirects, 3 >
{
	redirects &cin = std::array< redirects, 3 >::operator[]( STDIN_FILENO ), 
		&cout = std::array< redirects, 3 >::operator[]( STDOUT_FILENO ), 
		&cerr = std::array< redirects, 3 >::operator[]( STDERR_FILENO );

	std::vector< std::string > environment;
	
	int expected_return_code = 0;
	bool throw_on_unexpected_return_code = true;
};

template < typename Container, typename F >
void for_each( Container &c, F &&f )
{
	std::for_each( std::begin( c ), std::end( c ), std::forward< F >( f ) );
}

struct unexpected_return_code : std::exception
{
	unexpected_return_code( int c ) noexcept :
		code( c ) {}
		
	int code;
	const char* what() const noexcept override
	{
		return "unexpected_return_code";
	}
};

template < typename T = std::vector< std::string > >
T environment( char **environment )
{
	T result;
	while ( *environment )
	{
		result.push_back( *environment++ );
	}
	return result;
}

template < typename ...Args >
process::handle process( process::options options_, const std::string &cmd, Args ...args )
{
	constexpr auto input = unix_tools::pipe::input;
	constexpr auto output = unix_tools::pipe::output;

	std::array< unix_tools::pipe, 3 > pipes;

	constexpr std::array FILE_DESCRIPTORS = {
		STDIN_FILENO,
		STDOUT_FILENO,
		STDERR_FILENO
	};

	static_assert( STDERR_FILENO < FILE_DESCRIPTORS.size() );

	const auto get_direction = [&]( auto id )
	{
		return ( id == STDIN_FILENO ) ? input : output;
	};

	for ( auto pipe_id : FILE_DESCRIPTORS )
	{
		switch ( options_[ pipe_id ] )
		{
			case process::redirects::pipe:
				pipes[ pipe_id ].open();
				break;
			case process::redirects::null:
				pipes[ pipe_id ][ input ] = unix_tools::file( "/dev/null", unix_tools::file::mode::read );
				pipes[ pipe_id ][ output ] = unix_tools::file( "/dev/null", unix_tools::file::mode::write );
				break;
			case process::redirects::parent:
				break;
		}
	}
	
	const auto pid = check_errno( fork );
	if ( pid == 0 )
	{	
		for ( auto pipe_id : FILE_DESCRIPTORS )
		{
			if ( pipes[ pipe_id ] )
			{
				check_errno( close, pipe_id );
				check_errno( 
					dup2, 
					pipes[ pipe_id ][ get_direction( pipe_id ) ].get(),
					pipe_id
				);
				pipes[ pipe_id ].close();
			}
		};

		std::vector< std::string > arguments = { cmd, args... };
		std::vector< char* > parameters, environment;
		const auto append_null_terminator = []( auto &storage )
		{
			return [&]( auto &str )
			{
				str.push_back( 0 );
				storage.push_back( &str[ 0 ] );
			};
		};
		for_each( arguments, append_null_terminator( parameters ) );
		for_each( options_.environment, append_null_terminator( environment ) );
		
		parameters.push_back( nullptr );
		environment.push_back( nullptr );
		execve( cmd.c_str(), parameters.data(), environment.data() );
		exit( -1 );
	}
	
	constexpr auto get_status = []( int pid, options opt )
	{
		int status = 0;
		waitpid( pid, &status, 0 );
		if ( status != opt.expected_return_code )
		{
			if ( opt.throw_on_unexpected_return_code )
			{
				throw unexpected_return_code{ status };
			}
		}
		return status;
	};
	
	for ( auto pipe_id : FILE_DESCRIPTORS )
	{
		pipes[ pipe_id ][ get_direction( pipe_id ) ].reset();
	}
	
	return {
		pid,
		{
			options_.expected_return_code,
			std::async( get_status, pid, options_ )
		},
		std::move( pipes[ STDIN_FILENO ][ output ] ),
		std::move( pipes[ STDOUT_FILENO ][ input ] ),
		std::move( pipes[ STDERR_FILENO ][ input ] )
	};
}

}}}