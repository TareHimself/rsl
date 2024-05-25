layout(location=0) out vec2 oUV;
layout(set=1 , binding=1) uniform sampler2D ImageT;
layout(location=0) in vec2 iUV;
layout(location=1) out vec4 oColor;
void  main()
{
	generateRectVertex( pRect.size , ui.projection , pRect.transform , gl_VertexIndex , gl_Position , oUV );
	
}
