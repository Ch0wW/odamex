#==========================================
#
#	ODAMEX FOR PLAYSTATION VITA CMAKE FILE
#	
#	Do not forget to run this before even 
#	trying to do ANYTHING (CMake/Compile)!
#
#   Please read "client/psvita/readme.md" for 
#   more informations on how to compile for 
#   the platform.
#
#==========================================
message("Generating Makefile for Playstation Vita")
message("Please make sure you read client/psvita/readme.md first for compilation instructions !")
message("")
message("")

# Build type (Release/Debug)
set (CMAKE_BUILD_TYPE, "Debug")

# Project Settings
set(VITA_APP_NAME "Odamex for PSVita")
set(VITA_TITLEID  "ODA000666")
set(VITA_VERSION  "00.90")

# Odamex specific settings
set (BUILD_CLIENT 1)
set (BUILD_SERVER 0)
set (BUILD_MASTER 0)
set (BUILD_LAUNCHER 0)
set (USE_MINIUPNP 0)
set (ENABLE_PORTMIDI 0)

# This is a flag meaning we're compiling for a console
set (GCONSOLE 1)

# Since it's C++11...
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

function (odamex_target_settings_psvita BLOB)
    add_definitions("-DUNIX -DGCONSOLE -D__VITA__ -fpermissive")
endfunction()


# Target link project
macro(set_psvita_libraries PROJECT)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT} 
    # SDL2 MIXER
    SDL2_mixer
    vorbisfile 
    vorbis 
    ogg 
    FLAC 
    mikmod 
    mpg123

    #SDL2
    vita2d
    SDL2
    SceDisplay_stub
    SceCtrl_stub
    SceAudio_stub
    SceSysmodule_stub
    SceGxm_stub
    SceCommonDialog_stub
    SceAppMgr_stub
    SceTouch_stub
    SceHid_stub
    SceMotion_stub 
    ScePower_stub 
    SceAppUtil_stub
  
    # CURL
    SceIofilemgr_stub
    SceNet_stub
    SceNetCtl_stub
    SceHttp_stub
    SceSsl_stub
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}

    # Necessary
    c m z  )

endmacro()

function(create_package_psvita PROJECT)

# Include CMake for Vita
include("${VITASDK}/share/vita.cmake" REQUIRED)
vita_create_self(eboot.bin odamex)
vita_create_vpk(odamex.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${CMAKE_SOURCE_DIR}/client/psvita/sce_sys/icon0.png sce_sys/icon0.png
  FILE ${CMAKE_SOURCE_DIR}/client/psvita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE ${CMAKE_SOURCE_DIR}/client/psvita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE ${CMAKE_SOURCE_DIR}/client/psvita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
endfunction()