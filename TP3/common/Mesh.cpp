#include "Mesh.hpp"

void Mesh::compute_indices(){
    indices.resize(3 * triangles.size());
    int k = 0;
    for(int i = 0; i < triangles.size(); i ++){
        indices[k] = triangles[i][0]; 
        indices[k + 1] = triangles[i][1]; 
        indices[k + 2] = triangles[i][2]; 
        k += 3;
    }
}

Mesh::Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t){
    vertices = v;
    triangles = t;

    // compute_indices();
}

Mesh::Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t, std::vector<glm::vec2> uvs){
    vertices = v;
    triangles = t;
    uv = uvs;
    compute_indices();
}

Mesh::Mesh(){}

std::vector<unsigned short> Mesh::getIndices(){
    return indices;
}

void Mesh::loadToGpu(GLuint prog_id){
    glUseProgram(prog_id);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vertex);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );


    // UVs buffer                   // only for PLANE
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.uv);
    glVertexAttribPointer(
            1,                  // attribute
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );

    // glDisableVertexAttribArray(0);
        
}

void Mesh::draw(){

    // Draw the triangles !
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.element);
    glDrawElements(
            GL_TRIANGLES,      // mode
            indices.size(),    // count            // SOUP for plane, INDICES for chairs
            GL_UNSIGNED_SHORT,   // type
            (void*)0           // element array buffer offset
    );


    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

