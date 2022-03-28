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

//vao + vbos
struct Buffer{
    GLuint vertex;
    GLuint normal;
    GLuint element;
    GLuint uv;

    GLuint vao;
};

// Triangle struct (calculs de normales + point dans triangle)
struct Triangle{
    glm::vec3 a, b, c;
    float area;
    glm::vec3 normal;

    void compute_area_normal(){
        glm::vec3 e1 = a - b;
        glm::vec3 e2 = a - c;

        glm::vec3 not_normalized = glm::cross(e1, e2);
        normal = glm::normalize(not_normalized);

        area = not_normalized.length() / 2.f;
    }  

    bool point_in_triangle(const glm::vec3 p)
    {   
        // point directement dans la triangulation
        if(p == a || p == b || p == c)
        {   
            return true;
        }

        glm::vec3 _a = p-a;
        glm::vec3 _b = p-b;
        glm::vec3 _c = p-c;
        
        //produit vectoriel nul si vecteurs colinéaires 
        glm::vec3 pbc_normal = glm::cross(_b,_c);
        glm::vec3 pca_normal = glm::cross(_c,_a);
        glm::vec3 pab_normal = glm::cross(_a,_b);

        //le point ne peut pas être contenu dans le triangle 
        glm::vec3 null_vec(0,0,0);
        if(pbc_normal == null_vec || pca_normal == null_vec || pab_normal == null_vec)
            return false;

        pbc_normal = glm::normalize(pbc_normal);
        pca_normal = glm::normalize(pca_normal);
        pab_normal = glm::normalize(pab_normal);

        if(pbc_normal != pca_normal)
        {
            return false;
        }
        else if(pbc_normal != pab_normal)
        {
            return false;
        }
        return true;
    }
    
    Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) : a(v1), b(v2), c(v3){
        compute_area_normal();
    }

};

//Axis Aligned bounding box
struct AABB{
    glm::vec3 origin;
    glm::vec3 extents;

    AABB(): origin(glm::vec3(0,0,0)), extents(glm::vec3(1,1,1)){}

    // construction d'une bounding box à partir d'un point min et max
    AABB(glm::vec3 min, glm::vec3 max){
        origin = 0.5f * (min + max);
        extents = 0.5f * (max - min);
    }

    glm::vec3 getMin(){
        glm::vec3 p1 = origin + extents; 
        glm::vec3 p2 = origin - extents;

        return glm::vec3(
            fminf(p1.x, p2.x),
            fminf(p1.y, p2.y),
            fminf(p1.z, p2.z)
        ); 
    }

    glm::vec3 getMax(){
        glm::vec3 p1 = origin + extents; 
        glm::vec3 p2 = origin - extents;

        return glm::vec3(
            fmaxf(p1.x, p2.x),
            fmaxf(p1.y, p2.y),
            fmaxf(p1.z, p2.z)
        ); 
    }

};

class Mesh{
    protected:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uv;
        std::vector<unsigned short> indices; //Triangles concaténés dans une liste (soupe)
        std::vector<std::vector<unsigned short>> triangles;
        std::vector<glm::vec3> normals;

        void compute_indices();
    
    public:
        AABB boundingBox;
        Buffer buffers; // buffers initialisés auparavant   

        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t);
        Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t, std::vector<glm::vec2> u);
        Mesh();

        void generatePlan(int , int , glm::vec3 , int);

        std::vector<glm::vec3> getVertices();
        std::vector<unsigned short> getIndices();
        std::vector<std::vector<unsigned short>> getTriangles();
        std::vector<glm::vec3> getNormals();
        std::vector<glm::vec2> getUv();

        void initBuffers();
        void loadToGpu();
        void draw();

        void renderBbox();

        void compute_normals(); 
        void compute_boundingBox(); 

        static Mesh simplify(int, Mesh);
        
};

#endif