#include "common.glslh"

// UV Coordinates from vertex shader
in vec2 fragTexCoord;

// Output color
out vec4 finalColor;

void main() {
    // Right eye position in world space
    vec3 rightEyePos = (invViewMatrix * vec4(eyeDistance/2., 0., 0., 1.)).xyz;

    vec2 uv = (invViewMatrix * vec4(fragTexCoord, 0.0, 1.0)).xy;
    finalColor = mix(texture(texture0, uv), texture(depthTexture, uv), fragTexCoord.x);
}