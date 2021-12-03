if( CMAKE_VERSION VERSION_LESS 3.21 )
	if( NOT PROJECT_NAME )
		set( PROJECT_IS_TOP_LEVEL TRUE )
	endif()
endif()

cmake_policy( SET CMP0048 NEW )

project( process VERSION 1.0.0 )

cmake_minimum_required( VERSION 3.19 )

include( FetchContent )

add_library( process INTERFACE )

target_compile_features( process
	INTERFACE
		cxx_std_20
)

target_include_directories( process
	INTERFACE
		include
)

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
			process
			Catch2
	)

	add_test( runtest runtest ${CMAKE_COMMAND} )

endif()
