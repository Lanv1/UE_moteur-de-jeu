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

struct Triangle2D{
    glm::vec2 a;
    glm::vec2 b;
    glm::vec2 c;
    float area;

    void compute_area(){
        glm::vec2 e1 = b - a;
        glm::vec2 e2 = c - b;

        // std::cout<<e1.x<<", "<<e1.y<<std::endl;
        // std::cout<<e2.x<<", "<<e2.y<<std::endl;

        area = glm::cross(glm::vec3(e1, 0), glm::vec3(e2, 0)).length() / 2.f;
    }

    void compute_barycentric(glm::vec2 p, float* bary){
         //TODO Complete
        glm::vec2 e1 = b - a;
        glm::vec2 e2 = c - b;
        glm::vec2 e3 = a - c;

        glm::vec2 p1 = p - a;
        glm::vec2 p2 = p - b;
        glm::vec2 p3 = p - c;

        glm::vec3 cross_0 =  glm::cross(glm::vec3(e1, 0), glm::vec3(p1, 0));
        float area_0 = cross_0.length() / 2.0f;

        glm::vec3 cross_1 = glm::cross(glm::vec3(e2, 0), glm::vec3(p2, 0));
        float area_1 = cross_1.length() / 2.0f; 
        
        glm::vec3 cross_2 =  glm::cross(glm::vec3(e3, 0), glm::vec3(p3, 0));
        float area_2 = cross_2.length() / 2.0f; 

        std::cout<<"AREA0 "<<area_0<<std::endl;
        bary[2] = area_0 / this->area;
        bary[0] = area_1 / this->area;
        bary[1] = area_2 / this->area;

        

    }
    
    Triangle2D(glm::vec2 v1, glm::vec2 v2, glm::vec2 v3) : a(v1), b(v2), c(v3){
        compute_area();
    }

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
        void generatePlan(int , int , glm::vec3 , int);
        std::vector<glm::vec3> getVertices();
        std::vector<std::vector<unsigned short>> getTriangles();
        void initBuffers();
        void loadToGpu();
        void draw();
        
};

#endif