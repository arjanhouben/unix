if( CMAKE_VERSION VERSION_LESS 3.21 )
	if( NOT PROJECT_NAME )
		set( PROJECT_IS_TOP_LEVEL TRUE )
	endif()
endif()

cmake_policy( SET CMP0048 NEW )

project( unix_tools VERSION 1.0.0 )

cmake_minimum_required( VERSION 3.19 )

include( FetchContent )

add_library( unix_tools INTERFACE )

target_compile_features( unix_tools
	INTERFACE
		cxx_std_20
)

target_include_directories( unix_tools
	INTERFACE
		include
)

if( CMAKE_HOST_UNIX )
	find_package( Threads REQUIRED )
	target_link_libraries( unix_tools INTERFACE Threads::Threads )
endif()

if( PROJECT_IS_TOP_LEVEL )

	FetchContent_Declare( catch 
		GIT_REPOSITORY
			https://github.com/catchorg/Catch2.git
		GIT_TAG
			v2.13.7
	)

	FetchContent_Makeavailable( catch )

	enable_testing()

	find_program( CAT_COMMAND cat REQUIRED )
	find_program( ENV_COMMAND env REQUIRED )

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/test_main.in
		${CMAKE_CURRENT_BINARY_DIR}/test_main.cpp
	)

	add_executable( runtest
		tests.cpp
		${CMAKE_CURRENT_BINARY_DIR}/test_main.cpp
	)

	target_link_libraries( runtest
		PRIVATE
			unix_tools
			Catch2
	)

	add_test( runtest runtest ${CMAKE_COMMAND} )

endif()
