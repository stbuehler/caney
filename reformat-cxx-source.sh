#!/bin/sh

for f in $(find . -name '*.cpp' -o -name '*.hpp'); do clang-format-3.8 -i "$f"; done
