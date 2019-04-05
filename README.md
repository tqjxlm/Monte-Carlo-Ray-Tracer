# MT-Tracer

A CPU renderer using Monte-Carlo ray tracing algorithm.

It's a course assignment for Computer Graphics.

Tested under Windows 10, Visual Studio 2015 x64, while it is theoretically cross-platform.

## Features

* Ray tracing: Monte-Carlo strategy with stratified sampling and weighted cosine hemisphere sampling
* Lighting: native Blinn Phong model with reflection and refraction
* Acceleration: KDTree with AABB, OpenMP
* Material: Lambertian material support only
* Configurable: resolution, ray depth and ray density can be configured as needed

### File Format

__Notice__: an arbitrary obj model may not work for this project.

The program can load simple obj models, but the *.mtl format has to be modified for use in global illumination:

* Ks for reflection rate
* Ka for specular rate
* Tf for opacity

### Dependencies

* glm
* [tiny obj loader](https://github.com/syoyo/tinyobjloader)

All the dependencies above are head-only and already included in the repository.

You may need to use another version of GLM if your building environment is different from mine (VS 2015 X64)

## Usage

* Use CMake (3.9+) to configure the project
* Modify the render settings in Main.cpp
* Compile
* Run and wait (patience!)
* The output image will be at the working directory, i.e. the build directory specified in CMake

If you find the result unsatisfying, just increment the RAYS_PER_PIXEL variable in Main.cpp. You'll wait much longer for a better result.

## Demos

The following results are rendered with resolution of 1024 * 1024 and a maximum ray depth of 5.

scene 0: 128 rays / pixel

![image not available](samples/house_128.png)

scene 1: 1024 rays / pixel

![image not available](samples/scene1_1024.png)

scene 2: 512 rays / pixel

![image not available](samples/scene2_512.png)

scene 3: 128 rays / pixel

![image not available](samples/scene3_128.png)

scene 4: 256 rays / pixel

![image not available](samples/scene4_256.png)