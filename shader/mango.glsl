#version 330

// UV Coordinates from vertex shader
in vec2 fragTexCoord;

// Output color
out vec4 finalColor;

void main() {
	finalColor = vec4(fragTexCoord, 0.0, 1.0);
}