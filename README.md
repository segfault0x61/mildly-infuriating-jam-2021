# Mildly Infuriating Jam 2021
A fun but frustrating platformer game made for the Mildly Infuriating Jam 2021. Where you play as a vampire who transforms into a bat to platform your way around frustrating obstables.

## Setup
### Linux
Install dependencies.
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```
Then you can build and run it as such.
```bash
make
make run
```
### Windows
Install the following tools/dependencies.
1. Install Git Bash: https://git-scm.com/downloads
2. Install MingGW-w64: http://mingw-w64.org/doku.php
3. Download and install the SDL2 dev libraries for MinGW and extract them into the MinGW installation folder, e.g. C:/../MinGW/
4. Add the MinGW and the MinGW/bin folders to your PATH.
Then you can build it and run it as such.
```bash
mingw32-make
mingw32-make run
```
