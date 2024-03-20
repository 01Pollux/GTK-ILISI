// Signed distance to 2D line
float dline(vec2 p, vec2 a, vec2 b) {
    
    vec2 ba = b-a;
    vec2 n = normalize(vec2(-ba.y, ba.x));
    
    return dot(p, n) - dot(a, n);
    
}

// Signed distance to 2D box
float dbox(vec2 p, vec4 b) {
    
    p = abs(p - b.xy) - b.zw;
    return max(p.x, p.y);
    
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// Very very simple render example
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    fragColor = vec4(1.0);
}