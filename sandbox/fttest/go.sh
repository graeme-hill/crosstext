clang++ test.cpp \
    -o test \
    -I/usr/include/freetype2 \
    -I/usr/include/libpng \
    -L/usr/lib/x86_64-linux-gnu \
    -lfreetype \
    -lpng \
    -std=c++14
./test
