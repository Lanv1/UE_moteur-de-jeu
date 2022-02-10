#version 330 core

// coordonées uv
in vec2 UV;

in float height;
// Ouput data
out vec3 color;

uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D snowRock_texture;
// uniform sampler2D tex1;

void main(){
        if(height < 0.3){
                // color = texture(grass_texture, UV).rgb;
                color = mix(
                        texture(grass_texture, UV).rgb,
                        texture(rock_texture, UV).rgb,
                        0.5
                        );
        }else if(height < 0.7){
                color = texture(rock_texture, UV).rgb;
        }else {
                color = texture(snowRock_texture, UV).rgb;
        }


}
