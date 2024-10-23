struct QuadRenderInfo {
	ivec4 opts;
	vec4 color;
	vec4 borderRadius;
	vec2 size;
	mat3 transform;
	vec4 uv;
};
layout(set = 1 , binding = 0 , scalar) uniform _block_batch_info  {
	QuadRenderInfo quads[64];
} batch_info;
layout(push_constant , scalar) uniform constant {
	vec4 viewport;
	mat4 projection;
} push;
layout(location = 0) in vec2 iUV;
layout(location = 1) flat in int iQuadIndex;
layout(location = 0) out vec4 oColor;
float median(in float r , in float g , in float b)
{
	return max( min( r , g ) , min( max( r , g ) , b ) );
}
float screenPxRange(in vec2 uv , in vec2 size)
{
	vec2 unitRange = vec2( 30.0 ) / size;
	vec2 screenTexSize = vec2( 1.0 ) / fwidth( uv );
	return max( 0.5 * dot( unitRange , screenTexSize ) , 1.0 );
}
void main()
{
	QuadRenderInfo quad = batch_info.quads[iQuadIndex];
	vec4 pxColor = quad.color;
	int textureId = quad.opts.x;
	int mode = quad.opts.y;
	if(textureId != -1)
	{
		vec4 uvMapping = quad.uv;
		float u = mapRangeUnClamped( iUV.x , 0.0 , 1.0 , uvMapping.x , uvMapping.z );
		float v = mapRangeUnClamped( iUV.y , 0.0 , 1.0 , uvMapping.y , uvMapping.w );
		vec2 uv = vec2( u , v );
		if(mode == 0)
		{
			pxColor = pxColor * sampleTexture( textureId , uv );
		}
		else if(mode == 1)
		{
			vec2 texSize = getTextureSize( textureId );
			vec3 msd = sampleTexture( textureId , uv ).rgb;
			float sd = median( msd.r , msd.g , msd.b );
			float distance = screenPxRange( uv , texSize ) * ( sd - 0.5 );
			float opacity = clamp( distance + 0.5 , 0.0 , 1.0 );
			pxColor = mix( pxColor , vec4( pxColor.rgb , 0.0 ) , opacity );
		}
	}
	oColor = applyBorderRadius( gl_FragCoord.xy , pxColor , quad.borderRadius , quad.size , quad.transform );
}
