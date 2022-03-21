#version 330 core

// coordonées uv
in vec2 UV;


in float height;
in vec3 n;
out vec3 color;

uniform bool using_height;

void main(){
        if(using_height){
                //terrain
                color = vec3(height, height, height);
        }else{

                color = n;
        }

        
}
