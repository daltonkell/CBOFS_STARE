#/bin/bash

# Compile and link the demonstration CBOFS_STARE program.
# Since this is so simple, no Makefile is needed.
# If you have OpenMP and want to use it, use
#
# $ bash build.bash 1
#
# Otherwise, it will build without.

# Indicator variable to use OMP or not
HAVE_OMP=$1
CPPFLAGS="-I/home/dalton/SpatioTemporal/STARE/build/include/ -I/usr/local/include/ -I./include/"
CXXFLAGS="-std=c++11"
LDFLAGS="-L/home/dalton/SpatioTemporal/STARE/build/lib/ -L/usr/local/lib/"
CFLAGS="-fPIC"
WFLAGS="-Wall -Wpedantic"
DEBUG="-g -ggdb"

#  Compile:
if [[ "$HAVE_OMP" -eq 1 ]]; then
    echo "Building with OpenMP..."
    # This will give you a warning about ignored OMP pragmas
    g++ $CPPFLAGS $WFLAGS $DEBUG $CFLAGS $LDFLAGS -std=c++11 -o a.out CBOFS_STARE.cpp -lnetcdf -lSTARE
else
    # If using OMP, need to compile with -fopenmp then link
    g++ $CPPFLAGS $WFLAGS $DEBUG $CFLAGS $CFLAGS $CXXFLAGS -c CBOFS_STARE.cpp -o CBOFS_STARE.o -fopenmp
    g++ $CPPFLAGS $WFLAGS $DEBUG $CFLAGS $LDFLAGS $CXXFLAGS CBOFS_STARE.o -o a.out -fopenmp -lpthread -lnetcdf -lSTARE
fi
