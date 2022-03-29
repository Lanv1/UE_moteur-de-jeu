#ifndef PARTICLE_H
#define PARTICLE_H

#define MOVING 1
#define SLEEPING 0

#include "rigidbody.hpp"

class Particle : Rigidbody
{
    public:

        Particle();

        void update(float);
        void render();
        void applyForces();
        void solveConstraints(std::vector<AABB*>& constraints);

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();

        void setBounce(float b);
        float getBounce();

        // contact au repos -> arrêter de solver les contraintes
        int state;


    private:
        glm::vec3 position, oldPosition;
        glm::vec3 forces, velocity;  //Euler integration
        // glm::vec3 forces;           //Verlet integration
        float mass, bounce;

        glm::vec3 gravity;
        float friction;

};
#endif