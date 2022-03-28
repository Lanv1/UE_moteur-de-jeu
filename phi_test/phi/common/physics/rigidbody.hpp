#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <vector>
#include "rayGeo.hpp"
#include "../Mesh.hpp"

class Rigidbody
{
    public:

        Rigidbody(){};
        virtual ~Rigidbody(){};

        virtual void update(float deltaTime){};
        virtual void render(){};
        virtual void applyForces(){};
        virtual void solveConstraints(const std::vector<AABB>& constraints){};

};
#endif