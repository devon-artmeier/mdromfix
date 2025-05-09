@echo off
set OUT_PATH=out
set BUILD_PATH=%OUT_PATH%/build

if not exist "%OUT_PATH%" mkdir "%OUT_PATH%"

cmake -S . -B "%BUILD_PATH%"
cmake --build "%BUILD_PATH%" --config Release
cmake --install "%BUILD_PATH%" --config Release --prefix="%OUT_PATH%"

pause