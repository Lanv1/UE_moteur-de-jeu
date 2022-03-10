#version 330 core

// coordonées uv
in vec2 UV;

in float height;
out vec3 color;

uniform sampler2D sun_texture;
uniform sampler2D earth_texture;
uniform sampler2D moon_texture;

uniform int tex_to_use;

void main(){
        if(tex_to_use == 1){

                color = texture(earth_texture, UV).rgb;
        }else{

                color = texture(sun_texture, UV).rgb;
        }
       



}
