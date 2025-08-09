# Menu System
# Copyright (C) 2024-2025 komashchenko & Wend4r
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

project(mms2-menu_system
	VERSION 1.0.0
	DESCRIPTION "Menu System"
	HOMEPAGE_URL "https://github.com/Wend4r/mms2-menu_system"
	LANGUAGES C CXX
)

string(REGEX REPLACE "^[^-_]*[-_]([^-_]*)[-_]*(.*)" "\\1\\2" PROJECT_NAME_SUBSTRING "${PROJECT_NAME}")
string(TOLOWER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_UPPER)

set(PROJECT_OUTPUT_NAME "menu")

set(PROJECT_AUTHOR "Wend4r & komashchenko (Phoenix)")
set(PROJECT_DESCRIPTION_FULL "The menu system")
set(PROJECT_LICENSE "GPLv3")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SYMBOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sym")
set(SOURCE_MENU_DIR "${SOURCE_DIR}/menu")
set(SOURCE_MENU_PROVIDER_DIR "${SOURCE_MENU_DIR}/provider")
set(SOURCE_MENU_PROVIDER_GAMEDATA_DIR "${SOURCE_MENU_PROVIDER_DIR}/gamedata")
set(SOURCE_MENU_SCHEMA_DIR "${SOURCE_MENU_DIR}/schema")
set(SOURCE_MENU_SYSTEM_DIR "${SOURCE_MENU_DIR}/system")

function(set_or_external_dir VAR_NAME DEFAULT_DIR)
	if(${VAR_NAME})
		file(TO_CMAKE_PATH "${${VAR_NAME}}" ${VAR_NAME})
	else()
		set(${VAR_NAME} "${EXTERNAL_DIR}/${DEFAULT_DIR}")
	endif()

	set(${VAR_NAME} "${${VAR_NAME}}" PARENT_SCOPE)
endfunction()

set_or_external_dir(ANY_CONFIG_DIR "any_config")
set_or_external_dir(CONCAT_DIR "concat")
set_or_external_dir(DYNLIBUTILS_DIR "memory_utils")
set_or_external_dir(ENTITY_MANAGER_DIR "entity_manager")
set_or_external_dir(GAMEDATA_DIR "gamedata")
set_or_external_dir(LOGGER_DIR "logger")
set_or_external_dir(METAMOD_DIR "metamod-source")
set_or_external_dir(SOURCESDK_DIR "sourcesdk")
set_or_external_dir(TRNALSTIONS_DIR "translations")

include(cmake/platform/shared.cmake)

include(cmake/any_config.cmake)
include(cmake/concat.cmake)
include(cmake/dynlibutils.cmake)
include(cmake/entity_manager.cmake)
include(cmake/gamedata.cmake)
include(cmake/logger.cmake)
include(cmake/metamod.cmake)
include(cmake/sourcesdk.cmake)
include(cmake/translations.cmake)

set(SOURCE_MENU_PROVIDER_FILES
	${SOURCE_MENU_PROVIDER_GAMEDATA_DIR}/baseentity.cpp
	${SOURCE_MENU_PROVIDER_GAMEDATA_DIR}/baseplayerpawn.cpp
	${SOURCE_MENU_PROVIDER_GAMEDATA_DIR}/gamesystem.cpp
	${SOURCE_MENU_PROVIDER_GAMEDATA_DIR}/source2server.cpp
	${SOURCE_MENU_PROVIDER_GAMEDATA_DIR}/usercmd.cpp
)

set(SOURCE_MENU_SCHEMA_FILES
	${SOURCE_MENU_SCHEMA_DIR}/baseentity.cpp
	${SOURCE_MENU_SCHEMA_DIR}/basemodelentity.cpp
	${SOURCE_MENU_SCHEMA_DIR}/baseplayercontroller.cpp
	${SOURCE_MENU_SCHEMA_DIR}/baseviewmodel.cpp
	${SOURCE_MENU_SCHEMA_DIR}/bodycomponent.cpp
	${SOURCE_MENU_SCHEMA_DIR}/baseplayerpawn.cpp
	${SOURCE_MENU_SCHEMA_DIR}/baseplayerweaponvdata.cpp
	${SOURCE_MENU_SCHEMA_DIR}/csplayer_viewmodelservices.cpp
	${SOURCE_MENU_SCHEMA_DIR}/csplayerbase_cameraservices.cpp
	${SOURCE_MENU_SCHEMA_DIR}/csplayerpawnbase.cpp
	${SOURCE_MENU_SCHEMA_DIR}/csweaponbasevdata.cpp
	${SOURCE_MENU_SCHEMA_DIR}/gamescenenode.cpp
	${SOURCE_MENU_SCHEMA_DIR}/player_observerservices.cpp
	${SOURCE_MENU_SCHEMA_DIR}/player_weaponservices.cpp
	${SOURCE_MENU_SCHEMA_DIR}/pointworldtext.cpp
	${SOURCE_MENU_SCHEMA_DIR}/csplayerpawn.cpp
	${SOURCE_MENU_SCHEMA_DIR}/cpointorient.cpp
)

set(SOURCE_MENU_FILES
	${SOURCE_MENU_DIR}/chatcommandsystem.cpp
	${SOURCE_MENU_DIR}/chatsystem.cpp
	${SOURCE_MENU_DIR}/gameeventmanager2system.cpp
	${SOURCE_MENU_DIR}/pathresolver.cpp
	${SOURCE_MENU_DIR}/player.cpp
	${SOURCE_MENU_DIR}/profile.cpp
	${SOURCE_MENU_DIR}/profilesystem.cpp
	${SOURCE_MENU_DIR}/provider.cpp
)

set(SOURCE_FILES
	${SOURCE_DIR}/globals.cpp

	${SOURCE_MENU_PROVIDER_FILES}
	${SOURCE_MENU_DIR}/provider.cpp

	${SOURCE_MENU_SCHEMA_FILES}
	${SOURCE_MENU_DIR}/schema.cpp

	${SOURCE_MENU_FILES}
	${SOURCE_DIR}/menu.cpp
	${SOURCE_DIR}/menusystem_plugin.cpp # The shared entry point.
	${SOURCE_DIR}/menusystem_exports.cpp # Wrapper for C-like functions.
)

if(LINUX)
	set(LINK_OPTIONS
		${LINK_OPTIONS}

		-Wl,--version-script,${SYMBOLS_DIR}/version_script.lds
	)
endif()

set(INCLUDE_DIRS
	${INCLUDE_DIR}
	${PUBLIC_DIR}
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF

	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_OUTPUT_NAME})

if(WINDOWS)
	set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MACOS)
	set_target_properties(${PROJECT_NAME} PROPERTIES OSX_ARCHITECTURES "x86_64")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_OPTIONS} ${SOURCESDK_COMPILE_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${LINK_OPTIONS} ${SOURCESDK_LINK_OPTIONS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS} ${METAMOD_COMPILE_DEFINITIONS} ${SOURCESDK_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS} ${ANY_CONFIG_INCLUDE_DIRS} ${CONCAT_INCLUDE_DIRS} ${DYNLIBUTILS_INCLUDE_DIRS} ${ENTITY_MANAGER_INCLUDE_DIRS} ${GAMEDATA_INCLUDE_DIRS} ${LOGGER_INCLUDE_DIRS} ${METAMOD_INCLUDE_DIRS} ${SOURCESDK_INCLUDE_DIRS} ${TRNALSTIONS_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES} ${ANY_CONFIG_BINARY_DIR} ${CONCAT_BINARY_DIR} ${DYNLIBUTILS_BINARY_DIR} ${GAMEDATA_BINARY_DIR} ${LOGGER_BINARY_DIR} ${SOURCESDK_BINARY_DIR} ${TRNALSTIONS_BINARY_DIR})
