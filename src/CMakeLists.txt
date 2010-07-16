cmake_minimum_required (VERSION 2.6)

set (PROJECT 
	qrtcalarm)

include (SourceFiles.cmake)

set (TRANSLATIONS_PATH 
	./translations)

set (EXECUTABLE_OUTPUT_PATH ../bin)

project (${PROJECT})
include_directories (${CMAKE_BINARY_DIR} ./core ./gui)

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})
qt4_add_resources (QRC_SOURCES ${RESOURCES})
qt4_wrap_cpp (MOC_SOURCES ${HEADERS})
qt4_wrap_ui (UI_HEADERS ${FORMS})

foreach (LANGUAGE ${LANGUAGES})
	set (TS ${TRANSLATIONS_PATH}/${LANGUAGE}.ts)
	set (QM ${TRANSLATIONS_PATH}/${LANGUAGE}.qm)
	set (TRANSLATIONS ${TRANSLATIONS} ${TS})
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
	add_custom_command (
		OUTPUT ${TS}
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} ${FORMS} -ts ${TS})
	add_custom_command (
		OUTPUT ${QM}
		COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${QT_LRELEASE_EXECUTABLE} ${TS}
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

source_group ("Header Files" FILES ${HEADERS})
source_group ("Source Files" FILES ${SOURCES})
source_group ("Generated Files" FILES ${MOC_SOURCES})
source_group ("Resource Files" FILES ${QRC_SOURCES})

add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${UI_HEADERS} ${MOC_SOURCES} ${QRC_SOURCES} ${TRANSLATIONS})

target_link_libraries (${PROJECT} ${LIBRARIES} ${QT_LIBRARIES})
