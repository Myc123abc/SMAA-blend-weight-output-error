#version 460

#include "SMAA_frag.h"

void main()
{
	color = SMAANeighborhoodBlendingPS(texture_coord, offset[0], original_image, blend_texture);
}