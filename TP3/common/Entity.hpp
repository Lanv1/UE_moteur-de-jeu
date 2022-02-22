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
        const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

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

    void setRotation(float angle, const glm::vec3 axis){
       modelMatrix = glm::rotate(modelMatrix, angle, axis); 

    }
    void setRotation(float angle, int axis){
        rot[axis] = angle;
    }
    


};

class Entity {
    protected:

        Transform transform;
        Mesh entity_mesh;

        Entity* parent = nullptr;

    public:
        std::vector<Entity> children;
        
        Entity();
        
        void addMesh(Mesh m);
        void addTransformation(Transform t);

        Transform* getTransform();

        void addChild(Entity child);

        void updateSelfAndChild();
};
#endif