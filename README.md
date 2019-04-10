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

* Use CMake (3.9+) to configure the project
* Modify the render settings in Main.cpp
* Compile
* Run and wait (patience!)
* The output image will be at the working directory, i.e. the build directory specified in CMake

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

* Global BVH for acceleration.
  * Currently a KDTree is applied only to mesh level
* Light caching.
  * Previously lighted point need not to be calculated twice
* Use micro facet model instead of Blinn-Phong.
* Apply Schlicks approximation to mirror reflection.
* Unify mirror reflection and refraction.
* Strange GLM problem.
  * The latest version of GLM will ruin the whole program. Not sure why
