#include "Entity.hpp"

Entity::Entity(){

}

void Entity::addMesh(Mesh m){
    entity_mesh = m;
}

void Entity::addTransformation(Transform t){
    transform = t;
}