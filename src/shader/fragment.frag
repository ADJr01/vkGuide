#version 450
layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 finalColor; // defining final output color
void main(){
    finalColor = vec4(fragColor,1.0);
}