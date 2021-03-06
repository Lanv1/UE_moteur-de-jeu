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
glm::vec3 camera_position   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 1.;
float rota_speed = 1;
float camera_angle_X = 0.;
float camera_angle_Y = 0.;
vec3 orbital_axis;

//res
int resolution = 16;

bool updateMesh = false;
bool orbital = false;

float transl =0;

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
    window = glfwCreateWindow( 1024, 768, "TP3 - GLFW", NULL, NULL);
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
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "Model View Projection" matrices uniforms
    GLuint model_handle = glGetUniformLocation(programID, "model");
    GLuint view_handle = glGetUniformLocation(programID, "view");
    GLuint projection_handle = glGetUniformLocation(programID, "projection");
    GLuint using_tex_handle = glGetUniformLocation(programID, "tex_to_use");

    /****************************************/
    //Mesh ARRAYS
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> vert_uv;

    // HEIGHTMAPS/TEXTURES
    int sun_loc = glGetUniformLocation(programID, "sun_texture");
    int earth_loc = glGetUniformLocation(programID, "earth_texture");
    int moon_loc = glGetUniformLocation(programID, "moon_texture");

    glUseProgram(programID); //IMPORTANT
    GLuint tex0 = loadBMP_custom("../texture/sun.bmp", 1, sun_loc);
    GLuint tex1 = loadBMP_custom("../texture/earth.bmp", 2, earth_loc);
    GLuint tex2 = loadBMP_custom("../texture/moon.bmp", 3, moon_loc);

    //SPHERE MESH
    loadOFF("../OFF/sphere.off", indexed_vertices, indices, triangles);
    compute_sphere_uv(indexed_vertices, vert_uv);
    

    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint vertexbuffer;
    GLuint elementbuffer;
    GLuint uvbuffer;
    GLuint vertexbuffer_ch, elementbuffer_ch, uvbuffer_ch;
    GLuint vertexbuffer_ch_1, elementbuffer_ch_1, uvbuffer_ch_1;

    //ROOT ENTITY (SUN)
    Mesh sun(indexed_vertices, triangles, vert_uv);
    Entity root_entity(sun, (char *) "sun");
    sun.initBuffers();

    //CHILD_1 ENTITY (EARTH)
    Mesh earth(indexed_vertices, triangles, vert_uv);
    Entity child_1_entity(earth, (char *) "earth");
    earth.initBuffers();
    root_entity.addChild(child_1_entity);

    //CHILD_2 ENTITY (MOON)
    Mesh moon(indexed_vertices, triangles, vert_uv);
    Entity child_2_entity(moon, (char*) "moon");
    moon.initBuffers();
    child_1_entity.addChild(child_2_entity);
    

    // Modifier les parametres de base de la scene
    root_entity.transform.scale = vec3(5, 5, 5);
    root_entity.updateSelfAndChild();

    child_1_entity.transform.scale = vec3(0.2, 0.2, 0.2);
    child_1_entity.transform.setLocalPosition(vec3(-5, 0, 0));
    child_1_entity.updateSelfAndChild();

    child_2_entity.transform.setLocalPosition(vec3(2, 0, 0));
    child_2_entity.transform.scale = vec3(0.5, 0.5, 0.5);
    child_2_entity.updateSelfAndChild();

    
    //Naviguer a travers le SCENE GRAPH
    Entity* current_entity = &root_entity;
    while(current_entity != nullptr){
        std::cout<<current_entity->name<<" WORLD MODEL MATRIX: "<<std::endl;
        current_entity->transform.printModelMatrix();

        std::cout<<current_entity->name<<" LOCAL MODEL MATRIX"<<std::endl;
        current_entity->transform.printLocalModelMatrix();
        
        if(current_entity->hasChildren()){
            current_entity = current_entity->children.back();
        }else{
            current_entity = nullptr;
        }
    }   

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

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

        glm::mat4 View;
        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        View = glm::lookAt( camera_position, camera_target + camera_position, camera_up); 
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);
        View = glm::translate(View, vec3(0., 0., -8));
        View = glm::rotate(View, camera_angle_X, vec3(1, 0, 0));    // parametrage camera orbitale
        View = glm::rotate(View, camera_angle_Y, vec3(0, 1, 0));

        //PROJECTION et VIEW (caméra)
        glUniformMatrix4fv(projection_handle, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(view_handle, 1, GL_FALSE, &View[0][0]);

        //MODEL 
        glm::mat4 Model = root_entity.transform.modelMatrix;
        
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_tex_handle, 0);
        sun.loadToGpu();
        sun.draw();  


        //EARTH
        child_1_entity.transform.rot.y = angle;       // rotation autour du parent (soleil ici)
        child_1_entity.updateSelfAndChild();  
        Model = child_1_entity.transform.modelMatrix;
        Model = glm::rotate(Model, (float) 0.5, glm::vec3(1, 0, 0));
        Model = glm::rotate(Model, (float) angle, glm::vec3(0, 1, 0));
        
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_tex_handle, 1);                       // choix de la texture (0= soleil, 1= terre, 2= lune)
        earth.loadToGpu();
        earth.draw();   


        //MOON
        child_2_entity.transform.rot.y = 10.f * angle;
        child_2_entity.updateSelfAndChild();
        Model = child_2_entity.transform.modelMatrix;
        
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_tex_handle, 2);
        moon.loadToGpu();
        moon.draw();    

        angle += rota_speed;
        /****************************************/

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

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
        rota_speed += 0.01;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        rota_speed = (rota_speed - 0.01) < 0?rota_speed: rota_speed- 0.01;
    }


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera_position += cameraSpeed * camera_target;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera_position -= cameraSpeed * camera_target;

    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){

        camera_position -= glm::vec3(cameraSpeed, 0, 0) ;

    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){

        camera_position += glm::vec3(cameraSpeed, 0, 0);
 
    }

    if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
        camera_angle_Y -= cameraSpeed;        
    }

    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
        camera_angle_Y += cameraSpeed;        
    }
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
        camera_angle_X -= cameraSpeed;        
    }

    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
        camera_angle_X += cameraSpeed;        
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
