#/bin/bash
#
# Build the compiler
#
# You MUST replace the following commands with the commands for building your compiler

make clean -C ./src
find . -name "*.js" -type f
find . -name "*.js" -type f -delete
make -C ./src
