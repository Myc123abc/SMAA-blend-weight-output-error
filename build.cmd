cmake -B build -G Ninja .
cmake --build build

glslc -fshader-stage=vertex   shader/triangle.vert -o build/triangle_vert.spv
glslc -fshader-stage=fragment shader/triangle.frag -o build/triangle_frag.spv

glslc -fshader-stage=vertex   shader/SMAA_edge_detection.vert -o build/SMAA_edge_detection_vert.spv
glslc -fshader-stage=fragment shader/SMAA_edge_detection.frag -o build/SMAA_edge_detection_frag.spv
glslc -fshader-stage=vertex   shader/SMAA_blend_weight.vert -o build/SMAA_blend_weight_vert.spv
glslc -fshader-stage=fragment shader/SMAA_blend_weight.frag -o build/SMAA_blend_weight_frag.spv
glslc -fshader-stage=vertex   shader/SMAA_neighbor.vert -o build/SMAA_neighbor_vert.spv
glslc -fshader-stage=fragment shader/SMAA_neighbor.frag -o build/SMAA_neighbor_frag.spv