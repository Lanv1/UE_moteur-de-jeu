#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 UV;
out float height;
out vec3 n;

out vec3 height_offset;
void main(){

        gl_Position = (projection * view * model) * vec4(vertices_position_modelspace, 1);
        n = normal;
        UV = vertexUV;
      
}

