# Sega Genesis/Mega Drive ROM Fixer
[![Build Status](https://github.com/devon-artmeier/mdromfix/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/devon-artmeier/mdromfix/actions/workflows/cmake-multi-platform.yml)

This is a tool to pad/align and calculate checksum for Sega Genesis/Mega Drive ROMs.

## Usage

    mdromfix (-m) (-d) (-p [value]) [filename]
        -m         - Set mapper mode
        -d         - Don't apply padding (only alignment)
        -p [value] - Set padding/alignment value (0-255)
        [filename] - ROM filename

## Build Instructions

CMake is required to build this.

* On Windows, you can run "make.bat" and the built executable will be put in the "out/bin" folder.
* On other systems, you can call "make" and then "make install".
