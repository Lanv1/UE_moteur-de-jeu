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
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>

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
float angle = 0.;
float zoom = 1.;

//res
int resolution = 16;

bool updateMesh = false;



void generateVertices(std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &uv, int w, int h, vec3 pos, int res){
    float current_x, current_y;
    float x_step = (float) w / (float) res;
    float y_step = (float) h / (float) res;

    int array_index;
    vec3 vert;

    for(unsigned short i = 0; i < res; i ++){
        current_x = i * x_step;

        for(unsigned short j = 0; j < res; j ++) {

            array_index = (i * res + j);
            current_y = j*y_step;

            vert = vec3(current_x, current_y, 0);

            uv.resize(uv.size() + 1);
            uv[array_index] = vec2((float) i/ (float) res, (float) j/ (float)res);
            
            indexed_vertices.resize(indexed_vertices.size() + 1);
            indexed_vertices[array_index] = vert;

        }
    }
}

void randomizeHeight(std::vector<glm::vec3> &indexed_vertices, int bound) {
    srand(time(NULL));
    for(int i = 0; i < indexed_vertices.size();i ++){
        indexed_vertices[i][2] =  1. / (float) (1 + std::rand() % bound); 
        // std::cout<<vert[2]<<std::endl;
        // std::cout<< 1. / (float) (0.3 + std::rand() % bound)<<std::endl;
    }
}

void generatePlan(std::vector<std::vector<short unsigned int>> &triangles, std::vector<glm::vec3> &indexed_vertices, std::vector<glm::vec2> &uv, int w, int h, vec3 pos, int res) {
    // int res_x = 16;       //16x16 vertices
    // int res_y = 16;       //16x16 vertices
    triangles.clear();
    indexed_vertices.clear();
    uv.clear();

    int array_index;
    int tr_index = 0;

    generateVertices(indexed_vertices, uv, w, h, pos, resolution);

    
    for(unsigned short i = 0; i < resolution - 1; i ++){
        for(unsigned short j = 0; j < resolution - 1; j ++) {

            array_index = (i * resolution + j);
            
            triangles.resize(triangles.size() + 2);
            triangles[tr_index].resize(3);
            triangles[tr_index][0] = array_index; 
            triangles[tr_index][1] = array_index + 1; 
            triangles[tr_index][2] = array_index + resolution; 
            
            triangles[tr_index + 1].resize(3);
            triangles[tr_index + 1][0] = array_index + resolution;
            triangles[tr_index + 1][1] = array_index + 1;
            triangles[tr_index + 1][2] = array_index + 1 + resolution;

            tr_index += 2; 

        }
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
    window = glfwCreateWindow( 1024, 768, "TP2 - GLFW", NULL, NULL);
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

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "Model View Projection" matrices uniforms
    GLuint mvp_handle = glGetUniformLocation(programID, "mvp");

    //UV buffer
    GLuint uvbuffer;


    /****************************************/
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> vert_uv;

    // HEIGHTMAPS/TEXTURES
    // GLuint texture = loadBMP_custom("../texture/tex.bmp");
    // GLuint texture = loadBMP_custom("../heightMap/heightmap-1024x1024.bmp");
    // GLuint height_map0 = loadBMP_custom("../heightMap/heightmap-1024x1024.bmp");
    int height0_loc = glGetUniformLocation(programID, "height0");
    int tex0_loc = glGetUniformLocation(programID, "tex0");
    int height1_loc = glGetUniformLocation(programID, "height1");
    // int tex0_loc = glGetUniformLocation(programID, "tex1");

    glUseProgram(programID);
    // GLuint height_map1 = loadBMP_custom("../heightMap/Heightmap_Rocky.bmp", 0, height0_loc);
    GLuint height_map0 = loadBMP_custom("../heightMap/heightmap-1024x1024.bmp", 0, height0_loc);
    GLuint tex0 = loadBMP_custom("../texture/grass.bmp", 1, tex0_loc);

    
    generatePlan(triangles, indexed_vertices, vert_uv, 4, 4, vec3(0, 0, 0), resolution);
    // randomizeHeight(indexed_vertices, 10);

    indices.resize(3 * triangles.size());

    
    int k = 0;
    for(int i = 0; i < triangles.size(); i ++){
        indices[k] = triangles[i][0]; 
        indices[k + 1] = triangles[i][1]; 
        indices[k + 2] = triangles[i][2]; 
        k += 3;
    }


    // for(vec2 uv : vert_uv){
    //     std::cout<<uv[0]<<", "<<uv[1]<<std::endl;
    // }


    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Generate a buffer for UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, vert_uv.size() * sizeof(glm::vec2), &vert_uv[0], GL_STATIC_DRAW);
    
    // Load vertices into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);


    std::cout<<triangles.size()<<std::endl;
    std::cout<<indices.size()<<std::endl;
    
    
    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        
        if(updateMesh){
            generatePlan(triangles, indexed_vertices, vert_uv, 4, 4, vec3(0., 0., 0.), resolution);
            indices.clear();
            indices.resize(3 * triangles.size());
            
            k = 0;
            for(int i = 0; i < triangles.size(); i ++){
                indices[k] = triangles[i][0]; 
                indices[k + 1] = triangles[i][1]; 
                indices[k + 2] = triangles[i][2]; 
                k += 3;
            }

            std::cout<<triangles.size()<<std::endl;
            std::cout<<indices.size()<<std::endl;
            
            updateMesh = false;

            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, vert_uv.size() * sizeof(glm::vec2), &vert_uv[0], GL_STATIC_DRAW);
            
            // update mesh VBOs
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

        }
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

        // Use our shader
        glUseProgram(programID);


        // Model matrix : an identity matrix (model will be at the origin) then change
        glm::mat4 Model = glm::mat4(1.0f);
        
        Model =  glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
        
        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        glm::mat4 View = glm::lookAt( camera_position, camera_target + camera_position, camera_up);
        
        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);
        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms


        // Model =  glm::translate(Model, vec3(2, 2, 0));
        Model = glm::rotate(Model, angle, vec3(0., 1., 1));
        Model =  glm::translate(Model, vec3(-2, -2, 0));
        glm::mat4 mvp = Projection * View * Model;
        glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
        

        
        /****************************************/



        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
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
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                1,                  // attribute
                2,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
        );
        

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count            // SOUP for plane, INDICES for chairs
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteBuffers(1, &uvbuffer);
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

    //Camera zoom in and out
    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_position += cameraSpeed * camera_target;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_position -= cameraSpeed * camera_target;

    //TODO add translations
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){

        camera_position += glm::vec3(0, cameraSpeed, 0);
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){

        camera_position -= glm::vec3(0, cameraSpeed, 0);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){

        camera_position -= glm::vec3(cameraSpeed, 0, 0) ;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        camera_position += glm::vec3(cameraSpeed, 0, 0);

    }
    if(glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS){
        updateMesh = true;
        resolution = resolution-1 > 4? resolution-1: 4;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        updateMesh = true;
        resolution = resolution+1 < 240? resolution+1: 240;
    }
    

    //Big chair rotation
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        angle += cameraSpeed;

    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
