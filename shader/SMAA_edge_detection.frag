#version 460

#include "SMAA_frag.h"

layout(binding = 0) uniform sampler2D original_image;

void main()
{
  // TODO: try output image format r8g8 and change this vec4 to vec2
	color = vec4(SMAAColorEdgeDetectionPS(texture_coord, offset, original_image), 0.0, 0.0);
}