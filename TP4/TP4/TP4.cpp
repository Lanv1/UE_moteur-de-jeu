// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <time.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#include <common/Entity.hpp>
#include <common/Mesh.hpp>

GLFWwindow* window;
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
bool camera_TPS = false;
glm::vec3 camera_position   = glm::vec3(0.0f, 0.0f,  0.f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);
bool camera_moved = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 1.;
float rota_speed = 1;
float camera_angle_X = 0.;
float camera_angle_Y = -90.;

// ball movement
glm::vec3 ball_translation;
bool update_mvmt = true;
float obj_scale = 0.05;

// calcule les uv de la sphère et les stock dans uvs
void compute_sphere_uv(std::vector<glm::vec3> vertices, std::vector<glm::vec2>& uvs)
{
    float u,v;
    for(glm::vec3 vert : vertices)
    {
        u = atan2(vert.x, vert.z) / (2*M_PI) + 0.5;
        v = vert.y * 0.5 + 0.5;
        uvs.resize(uvs.size() + 1);
        uvs[uvs.size() - 1] = glm::vec2(u,v);
    }
}


/*******************************************************************************/

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP4 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);

    // VAO
    // GLuint VertexArrayID;
    // glGenVertexArrays(1, &VertexArrayID);
    // glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "Model View Projection" matrices uniforms
    GLuint model_handle = glGetUniformLocation(programID, "model");
    GLuint view_handle = glGetUniformLocation(programID, "view");
    GLuint projection_handle = glGetUniformLocation(programID, "projection");
    // GLuint using_tex_handle = glGetUniformLocation(programID, "tex_to_use");
    GLuint using_height_handle = glGetUniformLocation(programID, "using_height");
    GLuint ball_height_handle = glGetUniformLocation(programID, "ball_height_UV");

    /****************************************/
    //Mesh ARRAYS
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> vert_uv;

    // HEIGHTMAPS/TEXTURES
    int heightmap =glGetUniformLocation(programID, "height_sampler");

    glUseProgram(programID); //IMPORTANT

    GLuint height0 = loadBMP_custom("../heightMap/Heightmap_Mountain.bmp", 3, heightmap);

    //CUSTOM MESH
    loadOFF("../OFF/robot_wheeled.off", indexed_vertices, indices, triangles);
    compute_sphere_uv(indexed_vertices, vert_uv);
    

    float terrain_size = 5;
    Mesh terrain;
    terrain.generatePlan(terrain_size, terrain_size, vec3(0, 0, 0.), 100);
    Entity terrain_entity(terrain, (char *) "terrain");
    terrain.compute_normals();
    terrain.initBuffers();

    camera_position += vec3(0, terrain_size/5, terrain_size);

    //TERRAIN TRANSFORMATIONS SETUP
    terrain_entity.transform.rot.x = 90;
    terrain_entity.updateSelfAndChild();


    // MESH LOADED
    Mesh ball(indexed_vertices, triangles, vert_uv);
    Entity ball_entity;

    struct mesh_lod
    {
        Mesh low;
        Mesh medium;
        Mesh high;
    };

    //LOD SETUP
    mesh_lod ball_lod;
    ball_lod.low = Mesh::simplify(8, ball);
    ball_lod.medium = Mesh::simplify(15, ball);
    ball_lod.high = Mesh::simplify(32, ball);

    ball_lod.low.compute_normals();
    ball_lod.medium.compute_normals();
    ball_lod.high.compute_normals();

    ball_lod.low.initBuffers();
    ball_lod.medium.initBuffers();
    ball_lod.high.initBuffers();


    std::vector<glm::vec3> vertices = terrain.getVertices();
    std::vector<glm::vec2> terrain_uv = terrain.getUv();
    int tri_index;      

    // UVS des sommets composant le triangle
    float tri_uvs[6];   // 3 uvs
    

    glm::vec4 OBJ_POS;
    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    bool tst = true;
    do{
        
        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //MODEL
        glm::mat4 Model = terrain_entity.transform.modelMatrix;
        
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_height_handle, true);

        terrain.draw();
        
        // Calcul du triangle sur lequel l'objet est
        if(update_mvmt){

            tri_index = 0;   
            for(std::vector<unsigned short> tri : terrain.getTriangles()){
                glm::vec3 _v1(vertices[tri[0]].x, vertices[tri[0]].y, vertices[tri[0]].z);
                glm::vec3 _v2(vertices[tri[1]].x, vertices[tri[1]].y, vertices[tri[1]].z);
                glm::vec3 _v3(vertices[tri[2]].x, vertices[tri[2]].y, vertices[tri[2]].z);

                Triangle t(_v1, _v2, _v3);
                glm::vec4 ball_pos_matrix = ball_entity.transform.modelMatrix[3];
                glm::vec3 ball_pos(ball_pos_matrix.x, ball_pos_matrix.z, 0);    //Le terrain est un plan aligné sur les axes X,Z

                if(t.point_in_triangle(ball_pos)){

                    glm::vec2 U = terrain_uv[tri[0]];
                    glm::vec2 V = terrain_uv[tri[1]];
                    glm::vec2 W = terrain_uv[tri[2]];

                    tri_uvs[0] = U.x;
                    tri_uvs[1] = U.y;
                    tri_uvs[2] = V.x;
                    tri_uvs[3] = V.y;
                    tri_uvs[4] = W.x;
                    tri_uvs[5] = W.y;

                    //DEBUG
                    // for(int i = 0; i < 6; i++){
                    //     std::cout<<"UV["<<i<<"] = "<<tri_uvs[i]<<std::endl; 
                    // }
                }
                
                tri_index ++;

            }

            // UVs du triangle pour déterminer la hauteur dans le shader (heightmap).
            glUniform2fv(ball_height_handle, 3, &tri_uvs[0]);
        }
        
        // Déplacement de l'objet ou de la caméra: update du niveau de détails de l'objet
        if(update_mvmt || camera_moved){
            camera_moved = false;
            update_mvmt = false;

            OBJ_POS = ball_entity.transform.modelMatrix[3];
            float dist_cam_obj = sqrt((OBJ_POS.x - camera_position.x)*(OBJ_POS.x - camera_position.x) + (OBJ_POS.y - camera_position.y)*(OBJ_POS.y - camera_position.y) + (OBJ_POS.z - camera_position.z)*(OBJ_POS.z - camera_position.z));
            if(dist_cam_obj >= 8*(obj_scale*5)){
                ball_entity.setMesh(ball_lod.low);
            }else if(dist_cam_obj > 5*(obj_scale*5)){
                ball_entity.setMesh(ball_lod.medium);
            }else{
                ball_entity.setMesh(ball_lod.high);
            }
        
        }

        ball_entity.transform.setLocalPosition(ball_translation);
        ball_entity.transform.scale = glm::vec3(obj_scale, obj_scale, obj_scale);
        ball_entity.transform.rot.y = 90;
        ball_entity.updateSelfAndChild();

        Model = ball_entity.transform.modelMatrix;
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_height_handle, false);
        ball_entity.getMesh().draw();

        glm::mat4 View;
        //Caméra TPS fixée derrière au dessus de l'objet.
        if(camera_TPS){

            camera_position = glm::vec3(OBJ_POS.x, OBJ_POS.y, OBJ_POS.z) + 10*obj_scale * (glm::vec3(0, 0.2, 3));
            camera_target = glm::vec3(OBJ_POS) + glm::vec3(0., 0.4, 0.);
            View = glm::lookAt( camera_position, camera_target, camera_up); 
            // caméra inclinée à 45deg
            View = glm::translate(View, camera_position);    // parametrage camera orbitale
            View = glm::rotate(View, (float) 44.5,vec3(1, 0, 0));
            View = glm::translate(View, -camera_position);    // parametrage camera orbitale
        
        }else{
            // caméra libre
            glm::vec3 dir;
            dir.x = cos(glm::radians(camera_angle_Y)) * cos(glm::radians(camera_angle_X));
            dir.y = sin(glm::radians(camera_angle_X));
            dir.z = sin(glm::radians(camera_angle_Y)) * cos(glm::radians(camera_angle_X));

            camera_target = glm::normalize(dir);
            View = glm::lookAt(camera_position, camera_target + camera_position, camera_up); 
           
        }
        
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);

        //PROJECTION et VIEW (caméra)
        glUniformMatrix4fv(projection_handle, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(view_handle, 1, GL_FALSE, &View[0][0]);


        /****************************************/

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteProgram(programID);
    // glDeleteVertexArrays(1, &);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        ball_translation += glm::vec3(0, 0, -0.08);
        update_mvmt = true;        
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        ball_translation += glm::vec3(0, 0, 0.08);
        update_mvmt = true;        
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        ball_translation += glm::vec3(-0.08, 0, 0);
        update_mvmt = true;        
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        ball_translation += glm::vec3(0.08, 0, 0);

        update_mvmt = true;        
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera_position += cameraSpeed * camera_target;
        camera_moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera_position -= cameraSpeed * camera_target;
        camera_moved = true;

    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){

        // camera_position -= glm::vec3(cameraSpeed, 0, 0) ;
        camera_position -= glm::normalize(glm::cross(camera_target, camera_up)) * cameraSpeed;
        camera_moved = true;

    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        camera_position += glm::normalize(glm::cross(camera_target, camera_up)) * cameraSpeed;

        // camera_position += glm::vec3(cameraSpeed, 0, 0);
        camera_moved = true;

    }

    if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
        camera_angle_Y -= cameraSpeed * 20;
    }

    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
        camera_angle_Y += cameraSpeed * 20;        
    }
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
        camera_angle_X += cameraSpeed * 20;        
    }

    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
        camera_angle_X -= cameraSpeed * 20;        
    }

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
        camera_TPS = !camera_TPS;
    }

    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
