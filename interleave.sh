#!/bin/bash

mkdir -p build
clang interleave.c -o build/interleave
build/interleave

