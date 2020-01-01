#==========================================
#
#	ODAMEX FOR NINTENDO WIIU CMAKE FILE
#	
#	Do not forget to run this before even 
#	trying to do ANYTHING (CMake/Compile)!
#
#	export WUT_ROOT=$DEVKITPRO/wut
#	export PATH=/opt/devkitpro/devkitPPC/bin:/opt/devkitpro/tools/bin:$PATH
#
#	Then, when starting the CMAKE presentation...
#	cmake -DWIIU=1 -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/wut/share/wut.toolchain.cmake" ../odamex
#
#==========================================
message("Generating for Nintendo Wii U")
message("Please make sure you read wiiu.cmake first for compilation instructions !")
message("")
message("")

#Default variables to do
set (PKG_CONFIG_EXECUTABLE, "$ENV{DEVKITPRO}/portlibs/wiiu/bin/powerpc-eabi-pkg-config")
set (CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)

# Build type (Release/Debug)
set (CMAKE_BUILD_TYPE, "Release")

# Odamex specific settings
set (BUILD_CLIENT 1)
set (BUILD_SERVER 0)
set (BUILD_MASTER 0)
set (BUILD_ODALAUNCH 0)
set (USE_MINIUPNP 0)
set (USE_DISCORDRPC 0)
set (ENABLE_PORTMIDI 0)

# This is a flag meaning we're compiling for a console
set (GCONSOLE 1)