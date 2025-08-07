#version 330

// UV Coordinates from vertex shader
in vec2 fragTexCoord;

// UV Map to be used for transfer
uniform sampler2D texture0;

// Patter texture that should be mapped by UV texture
uniform sampler2D patternTex;

// Output color
out vec4 finalColor;

void main() {
	// UV coords need to be flipped because of some convention
	vec2 uv = texture(texture0, fragTexCoord).xy * vec2(1.0, -1.0) + vec2(0.0, 1.0);
	finalColor = texture(patternTex, uv);
}