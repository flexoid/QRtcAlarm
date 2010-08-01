cmake_minimum_required (VERSION 2.6)

set (PROJECT 
	qrtcalarm)

include (SourceFiles.cmake)

set (TRANSLATIONS_PATH 
	translations)

set (EXECUTABLE_OUTPUT_PATH ../bin)

project (${PROJECT})
include_directories (${CMAKE_BINARY_DIR} ./core ./gui)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})
qt4_add_resources (QRC_SOURCES ${RESOURCES})
qt4_wrap_cpp (MOC_SOURCES ${HEADERS})
qt4_wrap_ui (UI_HEADERS ${FORMS})

foreach (LANGUAGE ${LANGUAGES})
	set (TS ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATIONS_PATH}/${LANGUAGE}.ts)
	set (QM ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATIONS_PATH}/${LANGUAGE}.qm)
	set (TRANSLATIONS ${TRANSLATIONS} ${TS})
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
	if (NOT EXISTS ${TS})
		message(${TS})
		execute_process (
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} ${FORMS} -ts ${TS} -locations none)
	endif()
	add_custom_command (
		OUTPUT ${TS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} ${FORMS} -ts ${TS} -locations none)
	add_custom_command (
		OUTPUT ${QM}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS}
		MAIN_DEPENDENCY ${TS})
endforeach()

foreach (LIBRARY ${LIBRARIES})
	add_subdirectory (../${LIBRARY}/build bin/${LIBRARY})
endforeach ()

if (MSVC)
	add_definitions (/W3)
elseif (CMAKE_COMPILER_IS_GNUCXX)
	add_definitions (-Wall -pedantic)
else ()
	message ("Unknown compiler")
endif ()

# Generating "commit_hash.h" file with commit hash
find_package(Git)
if(GIT_FOUND)
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
		execute_process (
			COMMAND ${GIT_EXECUTABLE} log -1 --pretty="%h"
			OUTPUT_VARIABLE GIT_COMMIT_HASH
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		string(REGEX REPLACE "\"" "" GIT_COMMIT_HASH ${GIT_COMMIT_HASH})
	endif()
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/commit_hash.h.in ${CMAKE_CURRENT_SOURCE_DIR}/commit_hash.h)

add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${UI_HEADERS} ${MOC_SOURCES} ${QRC_SOURCES} ${TRANSLATIONS})

target_link_libraries (${PROJECT} ${LIBRARIES} ${QT_LIBRARIES})
