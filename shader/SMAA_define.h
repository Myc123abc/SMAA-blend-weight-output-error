//
// SMAA common header file
//

// push constant, use for smaa render target metrics
layout(push_constant) uniform PushConstant
{
	vec4 smaa_rt_metrics;
} pc;

#define SMAA_RT_METRICS pc.smaa_rt_metrics
#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA