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

layout(binding = 0) uniform sampler2D original_image;
layout(binding = 1) uniform sampler2D edges_texture;
layout(binding = 2) uniform sampler2D area_texture;
layout(binding = 3) uniform sampler2D search_texture;
layout(binding = 4) uniform sampler2D blend_texture;