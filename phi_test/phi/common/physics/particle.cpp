#include "particle.hpp"

void Particle::setPosition(const glm::vec3& pos)
{
    position = oldPosition = pos;
}

glm::vec3 Particle::getPosition()
{
    return position;
}

void Particle::setBounce(float b)
{
    bounce = b;
}

//bounce -> coefficient de restitution
float Particle::getBounce()
{
    return bounce;
}

//base particle
Particle::Particle()
{
    friction = 0.95f;
    gravity = glm::vec3(0.0f, -9.82f, 0.0f); //terre
    // gravity = glm::vec3(0.0f, -1.62f, 0.0f); //lune
    mass = 1.0f;
    bounce = 0.7f;
    state = MOVING;
}

void Particle::solveConstraints(std::vector<AABB*>& constraints)
{

    float sleeping_treshold = 0.0001f;

    for(AABB* bb : constraints)
    {
        Line traveled(oldPosition, position);
        Ray ray;

        if(lineTest(*bb, traveled))
        {
            glm::vec3 dir = glm::normalize(velocity);
            Ray ray(oldPosition, dir);
            RayIntersection intersection;

            if(rayCast(*bb, ray, &intersection))
            {
                position = intersection.point + intersection.normal * 0.003f;

                // Etat contact au repos pour rendre la particule stable lorsqu'elle est au contact de sa contrainte.
                if(traveled.sqLen() < sleeping_treshold || position == oldPosition)
                {
                    state = SLEEPING;   //si la particule est mise dans l'état repos avec la contrainte courante, ne pas tester les autres contraintes.
                    return; 
                }
                else
                {
                    glm::vec3 vn = intersection.normal * glm::dot(intersection.normal, velocity);   // décomposition en parallèle et perpendiculaire 
                    glm::vec3 vt = velocity - vn;
                    oldPosition = position;
                    velocity = vt - (vn * bounce);
                }
            }
        }
        else
        {
            state = MOVING; //pas de collision
        }
    }


}

void Particle::applyForces() {
    
    //Basique pour le moment (somme des forces appliquées sur la particule)
    forces = gravity * mass;

}

void Particle::update(float deltaT)
{

    if(state == MOVING)
    {

        oldPosition = position;

        // A = F / M
        glm::vec3 acceleration = forces * (1.0f / mass);

        glm::vec3 oldVelocity = velocity;   
        // V = V + A*dT (+ paramètre de friction)
        velocity = velocity * friction + acceleration * deltaT;

        // position = position + velocity * deltaT;                     //Euler integration (basique)
        position = position + (oldVelocity + velocity) * 0.5f * deltaT; // verlet integration (plus précis)
        

    }

}

//test
void Particle::render()
{
    //render une unique particule pour le moment
    float single_pt[3] = {position.x, position.y, position.z}; 

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &single_pt[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}
