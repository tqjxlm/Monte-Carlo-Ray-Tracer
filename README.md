# MT-Tracer

A CPU renderer using Monte-Carlo ray tracing algorithm.

It's a course assignment for Computer Graphics.

Tested under Windows 10, Visual Studio 2015/2017 x64, while it is theoretically cross-platform.

## Features

* Ray tracing: Monte-Carlo strategy with stratified sampling and weighted cosine hemisphere sampling
* Lighting: direct and indirect light with reflection and refraction
* Acceleration: KDTree, OpenMP
* Configurable: resolution, ray depth and ray density can be configured as needed

### Dependencies

* glm
* [tiny obj loader](https://github.com/syoyo/tinyobjloader)

All the dependencies above are head-only and already included in the repository.

 __Notice__: Due to an unknown bug, other versions of glm may not work with the code. Please use the provided one for now.

### File Format

The program can load simple obj models, but your mtl files may need modification to work. The following mtl properties are used:

* Kd - surface color or light color
* Ka - reflection rate (__different from the standard__)
* Ks - specular intensity
* Ke - emission intensity
* Ns - specular index
* d  - dissolve rate, i.e. opacity
* Ni - refraction index

## Usage

### Build

1. Use CMake (3.9+) to configure the project
2. Compile (use release build, unless there're run time issues)

### Run

* Arguments:
  * -s, --scene: scene ID (1-4), default 1
  * -d, --depth: ray depth, default 4
  * -r, --ray:   ray sample per pixel, default 4
  * -p, --pixel: image size, default 1024
* If running in Visual Studio, specify command line arguments in Debug Settings
* If running directly from the command line, please make sure the "resources" folder is in the same directory with the executive
* The output image will be at the working directory, i.e. the build directory specified in CMake or the executive directory

If you find the result unsatisfying, just increment the RAYS_PER_PIXEL variable in Main.cpp. You'll wait much longer for a better result.

## Demos

The following results are rendered with resolution of 1024 * 1024 and a maximum ray depth of 4.

scene 0: 128 rays / pixel

![image not available](samples/house_128.png)

scene 1: 1024 rays / pixel

![image not available](samples/scene1_1024.png)

scene 2: 256 rays / pixel

![image not available](samples/scene2_256.png)

scene 3: 256 rays / pixel

![image not available](samples/scene3_256.png)

scene 4: 256 rays / pixel

![image not available](samples/scene4_256.png)

## To Do

There are several known issues that may be improved in the future.

* BVH.
* Caching.
* PBR.
* Micro facet model.
* Fix the GLM version issue.
  * The latest version of GLM will ruin the whole program. Not sure why
