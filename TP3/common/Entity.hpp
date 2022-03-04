#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "Mesh.hpp"

struct Transform {
    //Local space information
    glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rot = { 0.0f, 0.0f, 0.0f }; //In degrees
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    //Global space information concatenate in matrix
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    //Dirty flag
    bool m_isDirty = true;

    glm::mat4 getLocalModelMatrix() {
    
        const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                    glm::radians(rot.x),
                    glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                    glm::radians(rot.y),
                    glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                    glm::radians(rot.z),
                    glm::vec3(0.0f, 0.0f, 1.0f));

        // Y * X * Z
        // const glm::mat4 rotationMatrix = transformY * transformX * transformZ;
        const glm::mat4 rotationMatrix = transformY;

        // translation * rotation * scale (also know as TRS matrix)
        return glm::translate(glm::mat4(1.0f), pos) *
                    rotationMatrix *
                    glm::scale(glm::mat4(1.0f), scale);
    }

    void computeModelMatrix(){
    
        modelMatrix = getLocalModelMatrix();
    }

    void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix){
            
        modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
        
    }

    void setLocalPosition(const glm::vec3& newPosition){
        pos = newPosition;
        m_isDirty = true;
    }

    void setLocalRotation(const glm::vec3& angles){
        
        rot = angles;
        m_isDirty = true;
    }


    void printLocalModelMatrix(){
        std::cout<<"MATRIX, pos="<<pos.x<<", "<<pos.y<<", "<<pos.z<<std::endl;
        glm::mat4 local = getLocalModelMatrix();
        for(int i = 0; i < 4; i ++){
            for(int j = 0; j < 4; j ++){
                std::cout<<local[i][j]<<" | ";
            }
            std::cout<<std::endl;
        }
    }

    void printModelMatrix(){
        std::cout<<"MATRIX, pos="<<pos.x<<", "<<pos.y<<", "<<pos.z<<std::endl;
        for(int i = 0; i < 4; i ++){
            for(int j = 0; j < 4; j ++){
                std::cout<<modelMatrix[i][j]<<" | ";
            }
            std::cout<<std::endl;
        }
    }
    


};

class Entity {
    protected:

        Mesh entity_mesh;


    public:
        std::vector<Entity*> children;
        Entity* parent = nullptr;
        
        Entity();
        
        void addMesh(Mesh m);
        void addTransformation(Transform t);

        Transform transform;

        void addChild(Entity& child);

        void updateSelfAndChild();
};
#endif