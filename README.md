# ScanLine

A CPU renderer using z-buffer scan line algorithm.

Course project for Computer Graphics.

## Features

Main feature: render 3d models using z-buffer scan line algorithm on CPU

* Support: camera manipulation
* Support: fps ~30 for small scenes
* Support: common model formats
* Support: texture
* Support: post-render effects using shaders (like anti-aliasing)
* Not support: lighting
* Not support: concurrency

## Dependencies

Prebuilt dependencies for Windows 10 x64 VS2015 are included in the repo.

* GLEW
* GLFW3
* SOIL
* ASSIMP

## Usage

1. Prepare dependencies if necessary
2. Use CMake to configure the project
3. Build (only tested on Win10 VS2015 x64)
4. Run the program given an integer [1-4] as argument, e.g. :

   ``` batch
   ./ScanLine.exe 2    #choose the 2nd prepared model
   ```

5. You can use mouse to rotate and zoom the model

## Basic Process

1. Read models as DrawableObject class
2. Use ZBufferScanLine class to rasterize the scene into a single quad of pixels
3. Render the quad on a window using a simple shader

## Demos

![image not available](https://s2.ax1x.com/2019/01/05/F7N9yD.png)
![image not available](https://s2.ax1x.com/2019/01/05/F7NpQO.png)
![image not available](https://s2.ax1x.com/2019/01/05/F7NSSK.png)