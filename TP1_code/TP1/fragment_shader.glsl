#version 330 core

// coordonées uv
in vec2 UV;

// Ouput data
out vec3 color;

uniform sampler2D mySampler;
void main(){

        color = texture(mySampler, UV).rgb;

}
