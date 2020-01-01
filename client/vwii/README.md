COMPILING ODAMEX ON WII

- Requirements :
- The latest release of DevKitPRO with the wii-development ticked.

- Always important after installing DEVKITPRO: make sure your building environment is always up to date.
> pacman -Syu

1) Install the following packages
> pacman -Sy ppc-portlibs ppc-pkg-config devkitpro-pkgbuild-helpers

2a) in a separate folder, clone the SDL-Wii github repository 
> Git clone https://github.com/dborth/sdl-wii 

2c) make, and install them.
> make && make install

3) Now, one directory ABOVE odamex , create a folder named "odamex_wii"
> mkdir odamex_wii 

it should go like that :

<YOUR FOLDER>
|- odamex
|- odamex_wii

4) Go to your odamex_wii folder and enter these commands

> cd odamex_wii
> source $DEVKITPRO/ppcvars.sh
> export PATH=/opt/devkitpro/devkitPPC/bin:/opt/devkitpro/tools/bin:$PATH

Please note those commands will need to be entered every time you launch msys2, so you could create a bash file for that.

5) CMake the project using the correct variables
> cmake -DVWII=1 -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/wii.cmake" ../odamex

6) Compile it !
> make
