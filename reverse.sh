#!/bin/bash

mkdir -p build
clang reverse.c -o build/reverse
build/reverse

