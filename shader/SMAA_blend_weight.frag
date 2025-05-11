#version 460

#include "SMAA_frag.h"

void main()
{
	color = SMAABlendingWeightCalculationPS(texture_coord, pixel_coord, offset, edges_texture, area_texture, search_texture, vec4(0.f));
}