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
#include <common/Entity.hpp>
#include <common/Mesh.hpp>
#include <common/physics/particle.hpp>

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

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "Model View Projection" matrices uniforms
    GLuint model_handle = glGetUniformLocation(programID, "model");
    GLuint view_handle = glGetUniformLocation(programID, "view");
    GLuint projection_handle = glGetUniformLocation(programID, "projection");
    GLuint bbox_handle = glGetUniformLocation(programID, "bbox");


    /****************************************/
    //Mesh ARRAYS
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> vert_uv;

    glUseProgram(programID); //IMPORTANT

    //CUSTOM MESH
    loadOFF("../OFF/robot_wheeled.off", indexed_vertices, indices, triangles);
    
    float terrain_size = 1;
    Mesh terrain;
    terrain.generatePlan(terrain_size, terrain_size, vec3(0, 0, 0.), 4);
    Entity terrain_entity(terrain, (char *) "terrain");
    terrain.compute_normals();
    terrain.compute_boundingBox();
    terrain.initBuffers();

    camera_position += vec3(0, terrain_size/5, terrain_size);


    //PARTICLE?
    Particle particle;
    std::vector<AABB*> constraints;
    // constraints.push_back(terrain.boundingBox);
    particle.setPosition(glm::vec3(0, 1, 0));
    particle.applyForces();

    std::cout<<"Alors ça bounce ?"<<particle.getBounce()<<std::endl;
    // terrain.renderBbox();

    // terrain_entity.transform.setLocalPosition(glm::vec3(0, -1, 0));
    terrain_entity.transform.rot.x = 90;
    terrain_entity.updateSelfAndChild();

    // // MESH LOADED
    Mesh ball(indexed_vertices, triangles, vert_uv);
    ball.compute_boundingBox();
    ball.compute_normals();
    ball.initBuffers();
    Entity ball_entity(ball, (char *) "ball");

    // constraints.push_back(&ball.boundingBox);
    constraints.push_back(&terrain.boundingBox);
    constraints.push_back(&ball.boundingBox);

    ball_entity.transform.scale = glm::vec3(0.2, 0.2, 0.2);
    ball_entity.updateSelfAndChild();

    glPointSize(30);

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
        // glm::mat4 Model = terrain_entity.transform.modelMatrix;
        ball_entity.transform.setLocalPosition(ball_translation);
        ball_entity.updateSelfAndChild();
        glm::mat4 Model = ball_entity.transform.modelMatrix;

        ball.boundingBox.applyTransformation(Model);
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(bbox_handle, 0);
        ball.draw();
        glUniform1i(bbox_handle, 1);

        Model = glm::mat4(1.f);
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        ball.renderBbox();

        Model = terrain_entity.transform.modelMatrix;
        terrain.boundingBox.applyTransformation(Model);
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(bbox_handle, 0);
        terrain.draw();

        glUniform1i(bbox_handle, 1);
        Model = glm::mat4(1.f);
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        terrain.renderBbox();


        // Model = glm::mat4(1.f);
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(bbox_handle, 0);

        particle.applyForces();
        particle.update(deltaTime);
        particle.solveConstraints(constraints);
        particle.render();
        
        // caméra libre
        glm::mat4 View;
        glm::vec3 dir;
        dir.x = cos(glm::radians(camera_angle_Y)) * cos(glm::radians(camera_angle_X));
        dir.y = sin(glm::radians(camera_angle_X));
        dir.z = sin(glm::radians(camera_angle_Y)) * cos(glm::radians(camera_angle_X));

        camera_target = glm::normalize(dir);
        View = glm::lookAt(camera_position, camera_target + camera_position, camera_up); 
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);

        //PROJECTION et VIEW (caméra)
        glUniformMatrix4fv(projection_handle, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(view_handle, 1, GL_FALSE, &View[0][0]);

        angle += 1;
        /****************************************/

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteProgram(programID);

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

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        rota_speed += 1;
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
