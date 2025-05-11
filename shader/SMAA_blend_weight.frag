#version 460

#include "SMAA_frag.h"

layout(binding = 1) uniform sampler2D edges_texture;
layout(binding = 2) uniform sampler2D area_texture;
layout(binding = 3) uniform sampler2D search_texture;

void main()
{
	color = SMAABlendingWeightCalculationPS(texture_coord, pixel_coord, offset, edges_texture, area_texture, search_texture, vec4(0.f));
}