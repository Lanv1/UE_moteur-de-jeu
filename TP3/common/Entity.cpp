#include "Entity.hpp"

Entity::Entity(){

}

void Entity::addMesh(Mesh m){
    entity_mesh = m;
}

void Entity::addTransformation(Transform t){
    transform = t;
}

void Entity::addChild(Entity& child){
    children.push_back(&child);
    child.parent = this;
}

void Entity::updateSelfAndChild(){
    if(parent != nullptr){
        // std::cout<<"I HAVE PARENT"<<std::endl;
        transform.computeModelMatrix(parent->transform.modelMatrix);
    }else {
        // std::cout<<"I DONT HAVE PARENT"<<std::endl;
        
        transform.computeModelMatrix();
    }

    for(Entity* child : children){
        child->updateSelfAndChild();
    }
}
