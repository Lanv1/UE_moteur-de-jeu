#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 vertexUV;

// Values that stay constant for the whole mesh.
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 UV;
uniform int tex_to_use;

uniform sampler2D sun_texture;
uniform sampler2D earth_texture;
uniform sampler2D moon_texture;


void main(){

        
        // TODO : Output position of the vertex, in clip space : MVP * position
        gl_Position = (projection * view * model) * vec4(vertices_position_modelspace, 1);
        UV = vertexUV;
}

