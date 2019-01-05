# MT-Tracer

A CPU renderer using Monte-Carlo ray tracing algorithm.

Course assignment for Computer Graphics.

## Features

* Support: simple obj models with well defined materials
* Support: reflection and refraction
* Support: KDTree and AABB acceleration
* Support: configuration for resolution, ray depth and ray density

## Dependencies

* glm
* [tiny obj loader](https://github.com/syoyo/tinyobjloader)

## Usage

* Use CMake to configure the project
* Modify the render settings in Main.cpp
* Compile (only tested for Win 10 VS2015 x64)
* Run and wait (patience!)
* The output image will be at the working directory (if you run in Visual Studio, it should be the project root)

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