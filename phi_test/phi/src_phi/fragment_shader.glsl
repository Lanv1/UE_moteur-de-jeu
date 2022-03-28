#version 330 core

// coordonées uv
in vec2 UV;


in float height;
in vec3 n;
out vec3 color;

uniform bool using_height;
uniform bool bbox;

void main(){
        if(bbox){
                color = vec3(1, 0, 0);
        }else{

                color = vec3(n.x, n.y, 0.2);
        }
        
}
