#ifndef MESH_H
#define MESH_H
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


class Mesh{
    protected:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uv;
        std::vector<unsigned short> indices; //Triangles concaténés dans une liste (soupe)
        std::vector<std::vector<unsigned short>> triangles;

        void compute_indices();
    
    public:
        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t);
        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t, std::vector<glm::vec2> u);
        Mesh();
        std::vector<unsigned short> getIndices();




};

#endif