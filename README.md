# Sega Genesis/Mega Drive ROM Fixer
[![Build Status](https://github.com/devon-artmeier/mdromfix/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/devon-artmeier/mdromfix/actions/workflows/cmake-multi-platform.yml)

This is a tool to pad and calculate checksum for Sega Genesis/Mega Drive ROMs.

## Usage

    mdromfix <-q> <-d> <-p [pad value]> [rom file]
    
        <-q>             - Quiet mode
        <-d>             - Don't pad
        <-p [pad value]> - Pad byte value
        [rom file]       - ROM file

## Build Instructions

CMake is required to build this.

* On Windows, you can run "make.bat" and the built executable will be put in the "out/bin" folder.
* On other systems, you can call "make" and then "make install".
