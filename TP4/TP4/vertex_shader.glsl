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

uniform bool using_height;
uniform vec2 ball_height_UV[3];

uniform sampler2D height_sampler;

out vec3 height_offset;
void main(){
        float amplitude = 2;
        float offset = -0.05;
        float sum_height = 0; //offset
        sum_height += texture(height_sampler, ball_height_UV[0]).r;
        sum_height += texture(height_sampler, ball_height_UV[1]).r;
        sum_height += texture(height_sampler, ball_height_UV[2]).r;
        sum_height /= 3.;
        
        height = texture(height_sampler,  vertexUV).r;
        height_offset = vec3(0., 0, 0);
        if(using_height){
                //terrain
                height_offset.z = height;
                sum_height = 0.;
                gl_Position = (projection * view * model) * vec4(vertices_position_modelspace + height_offset, 1);
        }else{

                //objet
                height_offset.z = height;                                      
                gl_Position = (projection * view * model) * vec4(vertices_position_modelspace, 1);
                gl_Position.y  -= amplitude * (offset + sum_height);
        }

        n = normal;
        UV = vertexUV;
      
}

