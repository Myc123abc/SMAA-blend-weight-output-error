//
// SMAA fragment shader common define
//

#include "SMAA_define.h"

#define SMAA_INCLUDE_VS 0
#include "SMAA.hlsl"

layout(location = 0) in vec2 texture_coord;
layout(location = 1) in vec2 pixel_coord;
layout(location = 2) in vec4 offset[3];

layout(location = 0) out vec4 color;