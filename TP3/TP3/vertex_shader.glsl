#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 vertexUV;
//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 UV;
out float height;

uniform sampler2D height0;

uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D snowRock_texture;


void main(){
        // height = texture(height0, vertexUV).r > texture(height1, vertexUV).r?texture(height0, vertexUV).r:texture(height1, vertexUV).r ;
        height = texture(height0, vertexUV).r;
        

        // TODO : Output position of the vertex, in clip space : MVP * position
        gl_Position = (projection * view * model) * vec4(vertices_position_modelspace + vec3(0, 0, height),1);
        UV = vertexUV;
}

