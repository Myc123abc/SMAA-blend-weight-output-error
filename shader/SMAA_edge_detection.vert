#version 460

#include "SMAA_vert.h"

void main()
{
	set_gl_Position_and_texture_coord();
	SMAAEdgeDetectionVS(texture_coord, offset);
}