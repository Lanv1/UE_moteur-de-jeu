#ifndef RAY_GEO_H
#define RAY_GEO_H

#include "../Mesh.hpp"
#include <cfloat>
#include <cmath>

#define CMP(x,y)                    \
    (fabsf((x)-(y)) <= FLT_EPSILON * \
        fmaxf(1.0f,                  \
        fmaxf(fabsf(x), fabsf(y)))   \
)

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    Ray(){}
    Ray(glm::vec3& o, glm::vec3& d): origin(o)
    {
        direction = glm::normalize(d);
    }
};

struct Line
{
    glm::vec3 start;
    glm::vec3 end;

    Line(){}
    Line(glm::vec3 a, glm::vec3 b): start(a), end(b){}

    float sqLen()
    {
        glm::vec3 v = end - start;
        return glm::dot(v,v);
    }
};

struct RayIntersection
{
    glm::vec3 point = glm::vec3(0,0,0);
    glm::vec3 normal = glm::vec3(0,0,1);
    float t = -1;
    bool hit = false;
};

float rayCast(AABB& aabb, const Ray& ray);

bool rayCast(AABB& aabb, const Ray& ray, RayIntersection* intersection);

bool lineTest(AABB& aabb, Line line);


#endif