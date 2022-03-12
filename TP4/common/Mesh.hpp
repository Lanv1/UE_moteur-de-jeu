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

struct Triangle{
    glm::vec3 a, b, c;
    glm::vec2 u, v, w;  //UV (for texture height)
    float area;
    glm::vec3 normal;

    void compute_area_normal(){
        glm::vec3 e1 = a - b;
        glm::vec3 e2 = a - c;

        // std::cout<<"E1 "<<e1.x<<", "<<e1.y<<std::endl;
        // std::cout<<"E2 "<<e2.x<<", "<<e2.y<<std::endl;
        glm::vec3 not_normalized = glm::cross(e1, e2);
        normal = glm::normalize(not_normalized);

        area = not_normalized.length() / 2.f;
        // std::cout<<"AREA "<<area<<std::endl;
    }  

    bool point_in_triangle(const glm::vec3 p)
    {   
        // point directement dans la triangulation
        if(p == a || p == b || p == c)
        {   
            return true;
        }

        // std::cout<<"P= "<<p.x<<", "<<p.y<<", "<<p.z<<std::endl;
        // std::cout<<"TRI NORMAL= "<<normal.x<<", "<<normal.y<<", "<<normal.z<<std::endl;
        glm::vec3 _a = p-a;
        glm::vec3 _b = p-b;
        glm::vec3 _c = p-c;
        // std::cout<<"A 1 "<<_a.x<<", "<<_a.y<<", "<<_a.z<<std::endl;
        // std::cout<<"B 2 "<<_b.x<<", "<<_b.y<<", "<<_b.z<<std::endl;
        // std::cout<<"C 3 "<<_c.x<<", "<<_c.y<<", "<<_c.z<<std::endl;

        //produit vectoriel nul si vecteurs colinéaires 
        glm::vec3 pbc_normal = glm::cross(_b,_c);
        glm::vec3 pca_normal = glm::cross(_c,_a);
        glm::vec3 pab_normal = glm::cross(_a,_b);

        //le point ne peut pas y être contenu dasn le triangle 
        glm::vec3 null_vec(0,0,0);
        if(pbc_normal == null_vec ||pca_normal == null_vec || pab_normal == null_vec)
            return false;

        pbc_normal = glm::normalize(pbc_normal);
        pca_normal = glm::normalize(pca_normal);
        pab_normal = glm::normalize(pab_normal);

        // std::cout<<"NORMAL 1 "<<pbc_normal.x<<", "<<pbc_normal.y<<", "<<pbc_normal.z<<std::endl;
        // std::cout<<"NORMAL 2 "<<pca_normal.x<<", "<<pca_normal.y<<", "<<pca_normal.z<<std::endl;
        // std::cout<<"NORMAL 3 "<<pab_normal.x<<", "<<pab_normal.y<<", "<<pab_normal.z<<std::endl;
        // std::cout<<std::endl;
        if(pbc_normal.z != pca_normal.z)
        {
            // std::cout<<"pbc normal: "<<pbc_normal.x<<", "<<pbc_normal.y<<", "<<pbc_normal.z<<std::endl;
            // std::cout<<"pca normal: "<<pca_normal.x<<", "<<pca_normal.y<<", "<<pca_normal.z<<std::endl;
            return false;
        }else if(pbc_normal != pab_normal)
        {
            return false;
        }
        return true;
    }

    // void compute_barycentric(glm::vec2 p, float* bary){
    //      //TODO Complete
    //     glm::vec2 e1 = b - a;
    //     glm::vec2 e2 = c - b;
    //     glm::vec2 e3 = a - c;

    //     glm::vec2 p1 = p - a;
    //     glm::vec2 p2 = p - b;
    //     glm::vec2 p3 = p - c;

    //     glm::vec3 cross_0 =  glm::cross(glm::vec3(e1, 0), glm::vec3(p1, 0));
    //     float area_0 = cross_0.length() / 2.0f;

    //     glm::vec3 cross_1 = glm::cross(glm::vec3(e2, 0), glm::vec3(p2, 0));
    //     float area_1 = cross_1.length() / 2.0f; 
        
    //     glm::vec3 cross_2 =  glm::cross(glm::vec3(e3, 0), glm::vec3(p3, 0));
    //     float area_2 = cross_2.length() / 2.0f; 

    //     std::cout<<"AREA0 "<<area_0<<std::endl;
    //     bary[2] = area_0 / this->area;
    //     bary[0] = area_1 / this->area;
    //     bary[1] = area_2 / this->area;  

    // }
    
    Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) : a(v1), b(v2), c(v3){
        compute_area_normal();
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
        std::vector<glm::vec2> getUv();
        void initBuffers();
        void loadToGpu();
        void draw();
        
};

#endif