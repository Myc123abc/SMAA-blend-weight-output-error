#version 460

#include "SMAA_frag.h"

layout(binding = 0) uniform sampler2D original_image;
layout(binding = 4) uniform sampler2D blend_texture;

void main()
{
	color = SMAANeighborhoodBlendingPS(texture_coord, offset[0], original_image, blend_texture);
}