#include <iostream>

#include "rsl/glsl.hpp"
#include "rsl/parser.hpp"
#include "rsl/tokenizer.hpp"
#include "rsl/utils.hpp"

int main() {
//     auto tokens = rsl::tokenize("<test>",R"(
// struct QuadRenderInfo
// {
//     int textureId;
//     int samplerId;
//     float4 color;
//     float4 borderRadius;
//     float2 size;
//     mat3 transform;
// };
//
//
// layout(set = 1,binding = 0, scalar) uniform batch_info {
//     float time;
//     float4 viewport;
//     mat4 projection;
//     QuadRenderInfo quads[1024];
// };
//
// @Vertex{
//
//     layout(location = 0) out float2 oUV;
//     layout(location = 1) out int oQuadIndex;
//
//     void main(){
//         int index = gl_VertexIndex;
//         int vertexIndex = mod(index,6);
//         int quadIndex = floor(x/y);
//
//         QuadRenderInfo renderInfo = batch_info.quads[quadIndex];
//
//         float4 extent = float4(0.0, 0.0, renderInfo.size);
//         float2 tl;
//         float2 tr;
//         float2 bl;
//         float2 br;
//
//         extentToPoints(extent, tl, tr, bl, br);
//
//         float2 vertex[] = { tl, tr, br, tl, br, bl };
//
//         float2 finalVert = doProjectionAndTransformation(vertex[vertexIndex], batch_info.projection, renderInfo.transform);
//
//         gl_Position = float4(finalVert, 0, 1);
//
//         float2 uvs[] = { float2(0.0), float2(1.0, 0.0), float2(1.0), float2(0.0), float2(1.0), float2(1.0, 0.0) };
//
//         oUV = vertex[vertexIndex] / size;
//
//         oQuadIndex = quadIndex;
//     }
//
//     
// }
//
//
// @Fragment{
//     layout (location = 0) in float2 iUV;
//     layout (location = 1,$flat) in int iQuadIndex;
//     layout (location = 0) out float4 oColor;
//
//     void main(){
//         oColor = batch_info.quads[iQuadIndex];
//     }
//
//     int foo(float[20] x) -> true ? 1 : 20;
// })");
    auto tokens = rsl::tokenize("<test>",R"(

struct QuadRenderInfo
{
    // [TextureId,RenderMode,0,0]
    int4 opts;
    float4 color;
    float4 borderRadius;
    float2 size;
    mat3 transform;
    float4 uv;
};


layout(set = 1,binding = 0, scalar) uniform batch_info {
    QuadRenderInfo quads[64];
};

push(scalar){
    float4 viewport;
    mat4 projection;
};

@Vertex{

    layout(location = 0) out float2 oUV;
    layout(location = 1) out int oQuadIndex;

    void main(){
        int index = gl_VertexIndex;
        int vertexIndex = int(mod(index, 6));
        int quadIndex = int(floor(index / 6));
        QuadRenderInfo quad = batch_info.quads[quadIndex];
        generateRectVertex(quad.size, push.projection, quad.transform, vertexIndex, gl_Position, oUV);
        oQuadIndex = quadIndex;
    }
}


@Fragment{
    layout (location = 0) in float2 iUV;
    layout (location = 1,$flat) in int iQuadIndex;
    layout (location = 0) out float4 oColor;

    float median(float r, float g, float b) {
        return max(min(r, g), min(max(r, g), b));
    }

    float screenPxRange(float2 uv,float2 size) {
        float2 unitRange = float2(30.0)/size;
        float2 screenTexSize = float2(1.0)/fwidth(uv);
        return max(0.5*dot(unitRange, screenTexSize), 1.0);
    }

    void main(){
        QuadRenderInfo quad = batch_info.quads[iQuadIndex];
        float4 pxColor = quad.color;
        int textureId = quad.opts.x;
        int mode = quad.opts.y;

        if(textureId != -1){
            float4 uvMapping = quad.uv;
            float u = mapRangeUnClamped(iUV.x,0.0,1.0,uvMapping.x,uvMapping.z);
            float v = mapRangeUnClamped(iUV.y,0.0,1.0,uvMapping.y,uvMapping.w);
            float2 uv = float2(u,v);

            if(mode == 0){
                pxColor = pxColor * sampleTexture(textureId,uv);
            } else if(mode == 1){
                float2 texSize = getTextureSize(textureId);
                float2 actualTexSize = texSize * (uvMapping.zw - uvMapping.xy);
                float3 msd = sampleTexture(textureId,uv).rgb;
                float sd = median(msd.r,msd.g,msd.b);
                float distance = screenPxRange(uv,actualTexSize)*(sd - 0.5);
                float opacity = clamp(distance + 0.5, 0.0, 1.0);
                oColor = mix(float4(pxColor.rgb,0.0),pxColor,opacity);
                return;
            }
        }

        oColor = applyBorderRadius(gl_FragCoord.xy, pxColor, quad.borderRadius, quad.size, quad.transform);
    }
})");
    auto ast = rsl::parse(tokens);
    //rsl::resolveIncludes(ast);
    rsl::resolveReferences(ast);
    auto str = rsl::glsl::generate(rsl::extractScope(ast,rsl::EScopeType::Fragment));
    std::cout  << str << std::endl;
}
