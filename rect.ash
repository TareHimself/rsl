@Fragment {
    
    void main() {
        vec2 unitRange = vec2(15)/vec2(push.rect.zw);
        vec2 screenTexSize = vec2(1.0)/fwidth(uv);
        return max(0.5*dot(unitRange, screenTexSize), 1.0);
    }
}