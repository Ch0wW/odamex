#==========================================
#
#	ODAMEX FOR NINTENDO SWITCH CMAKE FILE
#	
#	Do not forget to run this before even 
#	trying to do ANYTHING (CMake/Compile)!
#
#   Please read "client/switch/readme.md" for 
#   more informations on how to compile for 
#   the Nintendo Switch.
#
#==========================================
message("Generating for Nintendo Switch")
message("MAKE SURE you entered this command BEFORE either compiling OR CMAKEing!")
message("source $DEVKITPRO/switchvars.sh")
message("")
message("")

set (PKG_CONFIG_EXECUTABLE, "$ENV{DEVKITPRO}/portlibs/switch/bin/aarch64-none-elf-pkg-config")
set (CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)

# Build type (Release/Debug)
set (CMAKE_BUILD_TYPE, "Release")

# Since 0.8.3, Odamex is forced to used C++98 standards.
# Force it back to C++11 to allow the Switch to properly compile.
set(CMAKE_CXX_STANDARD 11)

# Odamex specific settings
set (BUILD_CLIENT 1)
set (BUILD_SERVER 0)
set (BUILD_MASTER 0)
set (BUILD_LAUNCHER 0)
set (USE_MINIUPNP 0)
set (USE_DISCORDRPC 0)
set (ENABLE_PORTMIDI 0)

# This is a flag meaning we're compiling for a console
set (GCONSOLE 1)

# NACP info
set (APP_TITLE "Odamex for Nintendo Switch")
set (APP_AUTHOR "The Odamex Team")
set (APP_VERSION "0.9.1")

set( CMAKE_CXX_EXTENSIONS ON )

# Compiler stuff
set(NACP_TOOL "${DEVKITPRO}/tools/bin/nacptool"  CACHE PATH "nacp-tool")
set(ELF_STRIP "${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-strip"  CACHE PATH "aarch64-none-elf-strip")
set(ELF2NRO_TOOL "${DEVKITPRO}/tools/bin/elf2nro"  CACHE PATH "elf2nro")

function (odamex_target_settings_nx _DIRECTORY _FILES)
#    set(${_DIRECTORY} ${CLIENT_DIR} switch)  # Nintendo Switch
#    file(${_FILES} CLIENT_HEADERS ${CLIENT_HEADERS} switch/*.h)
#    file(${_FILES} CLIENT_SOURCES ${CLIENT_SOURCES} switch/*.cpp)
    add_definitions("-DUNIX -DGCONSOLE")
endfunction()


# Create a function for post-compiling Odamex.
macro(odamex_target_postcompile_nx source)
  # Generate NACP
  add_custom_command(OUTPUT ${source}.nacp
    COMMAND ${NACP_TOOL} --create ${APP_TITLE} ${APP_AUTHOR} ${APP_VERSION} ${source}.nacp
    DEPENDS ${source}
    COMMENT "Generating NACP info for ${source}"
  )

  # Strip executable
  add_custom_command(OUTPUT ${source}_stripped
  COMMAND ${ELF_STRIP} -o ${CMAKE_BINARY_DIR}/client/odamex_stripped ${CMAKE_BINARY_DIR}/client/${source}
  DEPENDS ${source}
  COMMENT "Stripping data for ${source}"
)

  # NRO
  add_custom_command(OUTPUT ${source}.nro
    COMMAND ${ELF2NRO_TOOL} ${source}_stripped ${CMAKE_BINARY_DIR}/${source}.nro --icon=${CMAKE_SOURCE_DIR}/client/switch/assets/odamex.jpg --nacp=${source}.nacp
    DEPENDS ${source}.nacp ${source}_stripped
    COMMENT "Generating NRO ${source}.nro"
  )
  add_custom_target(NRO ALL DEPENDS ${source}.nacp ${source}.nro)
endmacro(switch_create_nro)