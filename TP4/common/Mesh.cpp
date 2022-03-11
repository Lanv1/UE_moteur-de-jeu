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

    compute_indices();
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

void Mesh::loadToGpu(){

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

void Mesh::initBuffers(){
    
    // Generate a buffer for UVs
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), &uv[0], GL_STATIC_DRAW);
    
    // Load vertices into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    buffers.element = elementbuffer;
    buffers.vertex = vertexbuffer;
    buffers.uv = uvbuffer;
}


// MISCELLANOUS: fonctions de génération de plan (vertices, triangles)
void generateVertices(std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &uv, int w, int h, glm::vec3 pos, int res){
    float current_x, current_y;
    float x_step = (float) w / (float) res;
    float y_step = (float) h / (float) res;

    int array_index;
    glm::vec3 vert;

    for(unsigned short i = 0; i < res; i ++){
        current_x = i * x_step;

        for(unsigned short j = 0; j < res; j ++) {

            array_index = (i * res + j);
            current_y = j*y_step;
            vert = glm::vec3(pos.x-(float) w/2, pos.y-(float) h/2, 0) + glm::vec3(current_x, current_y, 0);       // CENTERING VERTICES

            uv.resize(uv.size() + 1);
            uv[array_index] = glm::vec2((float) i/ (float) res, (float) j/ (float)res);
            
            indexed_vertices.resize(indexed_vertices.size() + 1);
            indexed_vertices[array_index] = vert;

        }
    }
}

void Mesh::generatePlan(int w, int h, glm::vec3 pos, int res) {
    // int res_x = 16;       //16x16 vertices
    // int res_y = 16;       //16x16 vertices
    triangles.clear();
    vertices.clear();
    uv.clear();

    int array_index;
    int tr_index = 0;

    generateVertices(vertices, uv, w, h, pos, res);

    
    for(unsigned short i = 0; i < res - 1; i ++){
        for(unsigned short j = 0; j < res - 1; j ++) {

            array_index = (i * res + j);
            
            triangles.resize(triangles.size() + 2);
            triangles[tr_index].resize(3);
            triangles[tr_index][0] = array_index; 
            triangles[tr_index][1] = array_index + 1; 
            triangles[tr_index][2] = array_index + res; 
            
            triangles[tr_index + 1].resize(3);
            triangles[tr_index + 1][0] = array_index + res;
            triangles[tr_index + 1][1] = array_index + 1;
            triangles[tr_index + 1][2] = array_index + 1 + res;

            tr_index += 2; 

        }
    }

    compute_indices();
}


