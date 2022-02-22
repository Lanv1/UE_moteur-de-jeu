#include "Entity.hpp"

Entity::Entity(){

}

void Entity::addMesh(Mesh m){
    entity_mesh = m;
}

void Entity::addTransformation(Transform t){
    transform = t;
}

Transform* Entity::getTransform(){
    return &transform;
}

void Entity::addChild(Entity child){
    children.push_back(child);
    child.parent = this;
}

void Entity::updateSelfAndChild(){
    if(parent != nullptr){
        this->transform.computeModelMatrix(parent->transform.modelMatrix);
    }else {
        this->transform.computeModelMatrix();
    }

    for(Entity child : children){
        child.updateSelfAndChild();
    }
}
