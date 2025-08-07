#version 330

// UV Coordinates from vertex shader
in vec2 fragTexCoord;

uniform float noiseSeed = 10.0;

// Output color
out vec4 finalColor;

void main() {
	// Random expression from here: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
	finalColor = vec4(vec3(fract(sin(dot(fragTexCoord + noiseSeed, vec2(12.9898, 4.1414))) * 43758.5453)), 1.0);
}