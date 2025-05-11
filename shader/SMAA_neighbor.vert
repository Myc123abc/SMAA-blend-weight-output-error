#version 460

#include "SMAA_vert.h"

void main()
{
	set_gl_Position_and_texture_coord();
	SMAANeighborhoodBlendingVS(texture_coord, offset[0]);
}