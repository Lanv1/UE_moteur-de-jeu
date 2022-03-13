#version 330 core

// coordonées uv
in vec2 UV;


in float height;
out vec3 color;

uniform sampler2D sun_texture;
uniform sampler2D earth_texture;
uniform sampler2D moon_texture;

uniform bool using_height;

void main(){
        if(using_height){
                //terrain
                color = vec3(height, height, height);
        }else{
                // color = texture(sun_texture, UV).rgb; 
                color = height * vec3(1, 0, 0); 
        }
        // color = 
        // color = height * vec3(0.2, 0, 0);
        
}
