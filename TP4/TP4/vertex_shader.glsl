#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 vertexUV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 UV;
out float height;
uniform int tex_to_use;
uniform bool using_height;
uniform vec2 ball_height_UV[3];

uniform sampler2D height_sampler;

out vec3 height_offset;
void main(){
        float amplitude = 4;
        float offset = -0.15;
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
                //balle
                height_offset.z = height;                                      
                gl_Position = (projection * view * model) * vec4(vertices_position_modelspace - amplitude*vec3(0, offset +sum_height,0) , 1);
        }
        UV = vertexUV;
      
}

