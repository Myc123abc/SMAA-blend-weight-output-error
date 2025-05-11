#version 460

vec2 vertices[] =
{
  { 0, -.5 },
  { .5, .5 },
  { -.5, .5 },
};

mat2 rotate(float degree)
{
  return mat2
  (
    cos(degree), sin(degree),
    -sin(degree), cos(degree)
  );
}

void main()
{
  gl_Position = vec4(rotate(45) * vertices[gl_VertexIndex], 0, 1);
}