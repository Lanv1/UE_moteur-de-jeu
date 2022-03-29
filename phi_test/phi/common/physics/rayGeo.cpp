#include "rayGeo.hpp"

// si t > 0 alors intersection
float rayCast(AABB& aabb, const Ray& ray)
{
    //Cyrus-Beck clipping
    glm::vec3 min = aabb.getMin();
    glm::vec3 max = aabb.getMax();

    float t1 = (min.x - ray.origin.x) / ray.direction.x;
    float t2 = (max.x - ray.origin.x) / ray.direction.x;
    float t3 = (min.y - ray.origin.y) / ray.direction.y;
    float t4 = (max.y - ray.origin.y) / ray.direction.y;
    float t5 = (min.z - ray.origin.z) / ray.direction.z;
    float t6 = (max.z - ray.origin.z) / ray.direction.z;

    float tmin = fmaxf(
                    fmaxf(
                        fminf(t1, t2),
                        fminf(t3, t4)
                    ),
                    fminf(t5, t6)
                );
    float tmax = fminf(
                    fminf(
                        fmaxf(t1, t2),
                        fmaxf(t3, t4)
                    ),
                    fmaxf(t5, t6)
                );

    if(tmax < 0)
    {
        // objet exactement derrière l'origine du ray
        return -1;
    }

    if(tmin > tmax)
    {
        //pas d'intersection
        return -1;
    }

    if(tmin < 0.0f)
    {
        // origine du ray DANS l'objet
        return tmax;
    }

    //rayon en dehors de l'objet, intersection en tmin
    return tmin;

         
}

bool rayCast( AABB& aabb, const Ray& ray, RayIntersection* intersection)
{
    //Cyrus-Beck clipping
    glm::vec3 min = aabb.getMin();
    glm::vec3 max = aabb.getMax();

    float t[6] = {0};

    //intersections possibles (eviter les divisions par 0 (remplacer 0 par un epsilon))
    t[0] = (min.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    t[1] = (max.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    t[2] = (min.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    t[3] = (max.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    t[4] = (min.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);
    t[5] = (max.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);;


    float tmin = fmaxf(
                    fmaxf(
                        fminf(t[0], t[1]),
                        fminf(t[2], t[3])
                    ),
                    fminf(t[4], t[5])
                );

    float tmax = fminf(
                    fminf(
                        fmaxf(t[0], t[1]),
                        fmaxf(t[2], t[3])
                    ),
                    fmaxf(t[4], t[5])
                );

    if(tmax < 0)
    {

        // objet exactement derrière l'origine du ray
        return false;
    }

    if(tmin > tmax)
    {
        //pas d'intersection
        return false;
    }

    float t_result = tmin;
    if(tmin < 0.0f)
    {
        // origine du ray DANS l'objet
        t_result = tmax;
    }


    intersection->t = t_result;
    intersection->hit = true;
    intersection->point = ray.origin + t_result * ray.direction;

    
    //normales de la bounding box
    glm::vec3 normals[] = {
        glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0),
        glm::vec3(0, -1, 0), glm::vec3(0, 1, 0),
        glm::vec3(0, 0, -1), glm::vec3(0, 0, 1)
    };

    for(int i = 0; i < 6; i ++)
    {
        if(CMP(t[i], t_result))
        {

            intersection->normal = normals[3];
        }
    }

    return true;

         
}


bool lineTest(AABB& aabb, Line line)
{
    Ray ray;
    ray.origin = line.start;
    ray.direction = glm::normalize(line.end - line.start);

    float t = rayCast(aabb, ray);

    return t >= 0 && (t*t) <= line.sqLen();
}
