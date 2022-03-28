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
    gravity = glm::vec3(0.0f, -9.82f, 0.0f);
    mass = 1.0f;
    bounce = 0.7f;
}

void Particle::solveConstraints(std::vector<AABB>& constraints)
{

    for(AABB bb : constraints)
    {
        Line traveled(oldPosition, position);
        Ray ray;
        // std::cout<<traveled.sqLen()<<std::endl;

        if(lineTest(bb, traveled))
        {
            glm::vec3 dir = glm::normalize(velocity);
            Ray ray(oldPosition, position);
            RayIntersection intersection;

            if(rayCast(bb, ray, &intersection) > 0)
            {
                std::cout<<"INTERSECTING A BB "<<intersection.t<<std::endl;
                position = intersection.point + intersection.normal * 0.002f;   //permet à la particule de rouler
                glm::vec3 vn = intersection.normal * glm::dot(intersection.normal, velocity);   // décomposition en parallèle et perpendiculaire 
                glm::vec3 vt = velocity - vn;
                std::cout<<"Intersection point: "<<intersection.point.x<<", "<<intersection.point.y<<", "<<intersection.point.z<<std::endl;
                oldPosition = position;
                // velocity = glm::vec3(0., 0., 0.);

                velocity = vt - vn * bounce;
            }
        }
    }

}

void Particle::applyForces() {
    //Basique pour le moment (somme des forces appliquées sur la particule)
    forces = gravity;
}

void Particle::update(float deltaT)
{
    oldPosition = position;
    // A = F / M
    glm::vec3 acceleration = forces * (1.0f / mass);

    // V = V + A*dT (+ paramètre de friction)
    velocity = velocity * friction + acceleration * deltaT;

    position = position + velocity * deltaT;
}

//test
void Particle::render()
{
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
