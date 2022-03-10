#include "Entity.hpp"

Entity::Entity(){

}

Entity::Entity(Mesh m, char* n): entity_mesh(m), name(n){}

void Entity::addMesh(Mesh m){
    entity_mesh = m;
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

bool Entity::hasChildren(){
    return children.size() > 0;
}

void Entity::setName(char* n){
    name = n;
}
