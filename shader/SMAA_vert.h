//
// SMAA vertex shader common define
//

#include "SMAA_define.h"

#define SMAA_INCLUDE_PS 0
#include "SMAA.hlsl"

layout(location = 0) out vec2 texture_coord;
layout(location = 1) out vec2 pixel_coord;
layout(location = 2) out vec4 offset[3];

//
// fullscreen triangle
//
vec2 vertices[] =
{
	{ -1, -1 },
	{  3, -1 },
	{ -1,  3 },
};

vec2 texture_coords[] = 
{
	{ 0, 0 },
	{ 2, 0 },
	{ 0, 2 },
};

void set_gl_Position_and_texture_coord()
{
	gl_Position   = vec4(vertices[gl_VertexIndex], 0, 1);
	texture_coord = texture_coords[gl_VertexIndex];
}