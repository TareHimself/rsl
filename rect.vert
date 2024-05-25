#version 450

layout(push_constant) uniform constant {

mat3 transform;

vec2 size;

float blurRadius;

vec4 tint;

} push;

layout(location=0) out vec2 oUV;

void  main()

{

generateRectVertex( push.size , ui.projection , push.transform , gl_VertexIndex , gl_Position , oUV );

}

