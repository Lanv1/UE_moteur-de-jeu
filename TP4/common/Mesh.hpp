#ifndef MESH_H
#define MESH_H
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

struct Buffer{
    GLuint vertex;
    GLuint element;
    GLuint uv;
};

class Mesh{
    protected:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uv;
        std::vector<unsigned short> indices; //Triangles concaténés dans une liste (soupe)
        std::vector<std::vector<unsigned short>> triangles;

        void compute_indices();         
    
    public:

        Buffer buffers; // buffers initialisés auparavant

        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t);
        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t, std::vector<glm::vec2> u);
        Mesh();

        std::vector<unsigned short> getIndices();
        void initBuffers();
        void loadToGpu();
        void draw();

        void generatePlan(int , int , glm::vec3 , int);
};

#endif