push() {
    mat3 transform;
    vec2 size;
    float blurRadius;
    vec4 tint;
};


@Vertex {
    layout(location = 0) out vec2 oUV;

    void main()
    {
        generateRectVertex(push.size,ui.projection,push.transform,gl_VertexIndex,gl_Position,oUV);
    }
}

@Fragment {
    layout(set = 1, binding = 1) uniform sampler2D ImageT[2000];

    layout (location = 0) in vec2 iUV;
    layout (location = 0) out vec4 oColor;

    void main() {
        vec4 pxColor = pRect.color;

        oColor = applyBorderRadius(gl_FragCoord.xy, pxColor,push.borderRadius,push.size,push.transform);
    }
}