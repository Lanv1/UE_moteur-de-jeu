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

