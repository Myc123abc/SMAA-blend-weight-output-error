#version 460

#include "SMAA_vert.h"

void main()
{
	set_gl_Position_and_texture_coord();
	SMAABlendingWeightCalculationVS(texture_coord, pixel_coord, offset);
}