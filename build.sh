#!/bin/sh

set -xe

mkdir -p ./bin/
cc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm
cc -Wall -Wextra -ggdb -o ./bin/gallary -Ithirdparty -I. examples/gallary.c
clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -o ./bin/triangle.wasm -I. ./examples/triangle.c
cc -Wall -Wextra -ggdb -I. -DSDL_PLATFORM -o ./bin/triangle ./examples/triangle.c -lm -lSDL2