#ifndef PARTICLE_H
#define PARTICLE_H

#include "rigidbody.hpp"

class Particle : Rigidbody
{
    public:

        Particle();

        void update(float);
        void render();
        void applyForces();
        void solveConstraints(std::vector<AABB>& constraints);

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();

        void setBounce(float b);
        float getBounce();


    private:
        glm::vec3 position, oldPosition;
        glm::vec3 forces, velocity;
        float mass, bounce;

        glm::vec3 gravity;
        float friction;

};
#endif