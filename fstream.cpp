#include "catch2/catch.hpp"
#include <fstream>
#include "arjan/posix/fstream.hpp"
#include "arjan/posix/stat.hpp"

extern const std::string cmake_command;

using mode = arjan::posix::file::mode;

constexpr auto test_path = "/tmp/stream_test.txt";
constexpr auto test_line = "this is a test";

TEST_CASE( "streams" )
{
	WHEN( "using a streambuf with default constructors" )
	{
		CHECK_THROWS(
			arjan::posix::ofstream(
				arjan::posix::file{}
			)
		);
	}
	WHEN( "creating a file and write a string to it" )
	{
		{
			arjan::posix::ofstream ofstream(
				arjan::posix::file( test_path, mode::write | mode::create | mode::truncate )
			);
			ofstream << test_line;
		}

		CHECK( arjan::posix::stat( test_path ).exists() );
		AND_WHEN( "we try to open the file and read the contents into a string" )
		{
			arjan::posix::ifstream ifstream(
				arjan::posix::file( test_path, arjan::posix::file::mode::read )
			);
			std::string line;
			CHECK( std::getline( ifstream, line ) );
			THEN( "the string should match the line we put in" )
			{			
				CHECK( line == test_line );
			}
		}
	}
}
