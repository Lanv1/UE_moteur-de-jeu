#include "Mesh.hpp"

void Mesh::compute_indices(){
    indices.resize(3 * triangles.size());
    int k = 0;
    for(int i = 0; i < triangles.size(); i ++){
        indices[k] = triangles[i][0]; 
        indices[k + 1] = triangles[i][1]; 
        indices[k + 2] = triangles[i][2]; 
        k += 3;
    }
}

Mesh::Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t){
    vertices = v;
    triangles = t;

    compute_indices();
}

Mesh::Mesh(std::vector<glm::vec3> v, std::vector<std::vector<unsigned short>> t, std::vector<glm::vec2> uvs){
    vertices = v;
    triangles = t;
    uv = uvs;
    compute_indices();
}

Mesh::Mesh(){}

std::vector<unsigned short> Mesh::getIndices(){
    return indices;
}

std::vector<std::vector<unsigned short>> Mesh::getTriangles(){
    return triangles;
}

std::vector<glm::vec3> Mesh::getVertices(){
    return vertices;
}

std::vector<glm::vec2> Mesh::getUv(){
    return uv;
}

void Mesh::loadToGpu(){

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vertex);
    glVertexAttribPointer(
            0,                  // attribute
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );


    // UVs buffer                   // only for PLANE
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.uv);
    glVertexAttribPointer(
            1,                  // attribute
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );

    // glDisableVertexAttribArray(0);
        
}

void Mesh::draw(){


    // Draw the triangles !
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.element);
    glDrawElements(
            GL_TRIANGLES,      // mode
            indices.size(),    // count            // SOUP for plane, INDICES for chairs
            GL_UNSIGNED_SHORT,   // type
            (void*)0           // element array buffer offset
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

}

void Mesh::initBuffers(){
    
    // Generate a buffer for UVs
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), &uv[0], GL_STATIC_DRAW);
    
    // Load vertices into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    buffers.element = elementbuffer;
    buffers.vertex = vertexbuffer;
    buffers.uv = uvbuffer;
}


// MISCELLANOUS: fonctions de génération de plan (vertices, triangles)
void generateVertices(std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &uv, int w, int h, glm::vec3 pos, int res){
    float current_x, current_y;
    float x_step = (float) w / ((float) res -1);
    float y_step = (float) h / ((float) res -1);

    int array_index;
    glm::vec3 vert;

    for(unsigned short i = 0; i < res; i ++){
        current_x = i * x_step;

        for(unsigned short j = 0; j < res; j ++) {
            array_index = (i * res + j);
            current_y = j*y_step;

            vert = glm::vec3(pos.x-((float) w/2), pos.y-((float) h/2), 0) + glm::vec3(current_x, current_y, 0);       // CENTERING VERTICES
            uv.resize(uv.size() + 1);
            uv[array_index] = glm::vec2((float) i/ (float) res, (float) j/ (float)res);
            
            indexed_vertices.resize(indexed_vertices.size() + 1);
            indexed_vertices[array_index] = vert;

        }
    }
}

void Mesh::generatePlan(int w, int h, glm::vec3 pos, int res) {
    // int res_x = 16;       //16x16 vertices
    // int res_y = 16;       //16x16 vertices
    triangles.clear();
    vertices.clear();
    uv.clear();

    int array_index;
    int tr_index = 0;

    generateVertices(vertices, uv, w, h, pos, res);

    
    for(unsigned short i = 0; i < res - 1; i ++){
        for(unsigned short j = 0; j < res - 1; j ++) {

            array_index = (i * res + j);
            
            triangles.resize(triangles.size() + 2);
            triangles[tr_index].resize(3);
            triangles[tr_index][0] = array_index; 
            triangles[tr_index][1] = array_index + 1; 
            triangles[tr_index][2] = array_index + res; 
            
            triangles[tr_index + 1].resize(3);
            triangles[tr_index + 1][0] = array_index + res;
            triangles[tr_index + 1][1] = array_index + 1;
            triangles[tr_index + 1][2] = array_index + 1 + res;

            tr_index += 2; 

        }
    }

    compute_indices();
}

glm::vec3 get_grid_indexes(glm::vec3 vertex, float min, float step)
{
    glm::vec3 result;
    int j = (vertex[0] - min)/step;
    int k = (vertex[1] - min)/step;
    int i = (vertex[2] - min)/step;
    result = glm::vec3(i, j, k);
    return result;
}

struct rep
{
    glm::vec3 pos;
    int size;
};

Mesh Mesh::simplify(int res, Mesh input)
{
    std::vector<glm::vec3> input_vert = input.vertices;
    std::vector<std::vector<unsigned short>> input_triangles = input.triangles;
    int input_sz = input_vert.size();
    int input_tri_sz = input_triangles.size();

    std::vector<glm::vec3> C;
    C.resize(8);

    float minVal = FLT_MAX;
    float minX = FLT_MAX;
    float minY = FLT_MAX;
    float minZ = FLT_MAX;

    float maxVal = 0;
    float maxX = 0;
    float maxY = 0;
    float maxZ = 0;

    // Calcul de la bounding box
    for(size_t i = 0; i < input_sz; i ++) {
        minX = std::min(minX, input_vert[i][0]);
        minY = std::min(minY, input_vert[i][1]);
        minZ = std::min(minZ, input_vert[i][2]);

        maxX = std::max(maxX, input_vert[i][0]);
        maxY = std::max(maxY, input_vert[i][1]);
        maxZ = std::max(maxZ, input_vert[i][2]);
    }
    
    minVal = std::min(minX, minY);
    minVal = std::min(minZ, minVal);

    maxVal = std::max(maxX, maxY);
    maxVal = std::max(maxZ, maxVal);

    maxVal += 0.05;
    minVal -= 0.05;
    
    C[0] = glm::vec3(minVal, minVal, minVal);
    C[1] = glm::vec3(minVal, maxVal, minVal);
    C[2] = glm::vec3(maxVal, maxVal, minVal);
    C[3] = glm::vec3(maxVal, minVal, minVal);

    C[4] = glm::vec3(minVal, minVal, maxVal);
    C[5] = glm::vec3(minVal, maxVal, maxVal);
    C[6] = glm::vec3(maxVal, maxVal, maxVal);
    C[7] = glm::vec3(maxVal, minVal, maxVal);

    std::vector<rep> repr_grid;
    repr_grid.resize(res * res * res);

    float step = (maxVal - minVal) / (float) (res - 1);

    // Construction de la grille des cellules des représentants
    for(size_t i = 0; i < res; i ++) {
        for(size_t j = 0; j < res; j ++) { 
            for(size_t k = 0; k < res; k ++) {  

                // Tableau 3D applati
                repr_grid[i*(res*res) + j*res + k].pos = glm::vec3(0., 0., 0.);
                repr_grid[i*(res*res) + j*res + k].size = 0;

            }
        }
    }  

    glm::vec3 grid_indexes;
    int grid_index;
    for(size_t i = 0; i < input_sz; i ++) {
        // Stockage des coordonées i,j,k de la grille pour obtenir le représentant de i
        grid_indexes = get_grid_indexes(input_vert[i], minVal, step);
        grid_index = (grid_indexes[0]*res*res) + (grid_indexes[1] * res) + grid_indexes[2];
        
        // Ajout du sommet courant a son représentant
        repr_grid[grid_index].pos += input_vert[i];
        repr_grid[grid_index].size += 1;
        // repr_grid[grid_index].normal += normals[i];

    }
    
    // Liste des représentants non nuls
    // repr.clear();
    // repr.resize(grid.size());
    
    std::vector<glm::vec3> repr;
    repr.resize(1);
    int insert_at = 0;
    // Stockage des représentants calculés dans repr (moyenne des sommets dans une même cellule)
    for(rep r : repr_grid)
    {
        if(r.size > 0)
        {
            repr[insert_at] = r.pos / (float) r.size;
            repr.resize(repr.size() + 1);
            insert_at ++;
        }
    }

    // Liste des triangles re-indexés (survivants)
    // std::vector< Triangle > surv; 
    std::vector<std::vector<unsigned short>> alive;
    std::vector<unsigned short> surv;
    surv.resize(3);
    glm::vec3 vert;
    int tri_index = 0;
    // Sélection des triangles à conserver
    for(std::vector<unsigned short> tri : input_triangles)
    {
        std::vector<int> repr_index;
        repr_index.clear();
        repr_index.resize(3);
        bool delete_t = false;

        for(int i = 0; i < 3; i ++)
        {
            vert = input_vert[tri[i]];
            grid_indexes = get_grid_indexes(vert, minVal, step);
            grid_index = (grid_indexes[0]*res*res) + (grid_indexes[1] * res) + grid_indexes[2];

            //cellule du représentant de vert dans le triangle tri
            repr_index[i] = grid_index;        
        }


        for(int i = 0; i < 3; i ++)
        {
           int comp_index = repr_index[i];
            for(int k = 0; k < 3; k ++)
            {
                if(comp_index == repr_index[k] && i != k)
                    delete_t = true;
            }   
        }

        if(delete_t)
        {
            // Elimination du triangle i
            for(int k = 0; k < 3; k ++)
                input_triangles[tri_index][k] = -1;
            
        }
        else
        {
            // Ré-indexer les sommets du triangle 
            for(int k = 0; k < 3; k ++){
                surv[k] = repr_index[k];
            }
            alive.push_back(surv);
        }
        tri_index ++;
    }

    // triangles = alive;
    // vertices = repr;

    return Mesh(repr, alive);
}

