# ReduceColours-c
A copy of my JavaScript reduce colour app but written in C.

This is a simple project I made for fun which uses OctTrees to store RGB colours from an image. It then uses the resulting OctTree to produce an optimised colour pallet which minimises the error between its self and the original image. Any number of colours can be specified and its more efficient to provide multiple at once so it can reuse the constructed tree.

See my much nicer version made in js: https://github.com/Cooldude2606/ReduceColours-js

## Installation

1) Download the project through your preferred method.
2) Download the two submodules in the lib dir, the make file does not do this for you.
3) Rename lodepng.cpp to lodeepng.c, the make file does not do this for you.
4) Run make to compile the app, it will produce two executables "app" and "multi"

App: This executable is the base one that only uses standard c headers so should work on all platforms

Multi: The only difference is that this uses posix threads to speed up image saving

### Examples

* `./app ./input.png 64` - Outputs an image and a pallet using only 64 colours
* `./app ./input.png 64,128,256` - Outputs three images and three pallets using only 64, 128, and 256 colours
* `./multi ./input.png 64,128,256` - Same as above but uses multiple threads to save the images faster