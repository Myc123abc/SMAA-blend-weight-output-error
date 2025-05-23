This project is a minimize code to solve SMAA blend weight output error leading SMAA output image uncomplete Anti-Aliasing.

External libraries as follow:
* https://github.com/libsdl-org/SDL release-3.2.12
* https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git v3.2.1
* https://github.com/iryoku/smaa.git master

Test use vulkan version 1.4.309

Windows 10.0.19045
64bit
Clang 18.1.8

It's over. Add `#define SMAA_FLIP_Y 0` is OK.

The detail reference: https://stackoverflow.com/questions/79615447/smaa-weight-blend-texture-is-unclear
