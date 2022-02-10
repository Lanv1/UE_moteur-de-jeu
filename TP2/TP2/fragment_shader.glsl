#version 330 core

// coordonées uv
in vec2 UV;

// Ouput data
out vec3 color;

uniform sampler2D tex0;
// uniform sampler2D tex1;

void main(){

        color = texture(tex0, UV).rgb;

}
