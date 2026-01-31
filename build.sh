rm -rf build/linux/*
cmake -S . -B build/linux -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build/linux