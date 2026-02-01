RMDIR /s /q "build/win32"
echo Removed previous build directory and starting new build...
cmake -S . -B build/win32 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/win32