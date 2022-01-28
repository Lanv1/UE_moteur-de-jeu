// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

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

void generateVertices(std::vector<glm::vec3> &indexed_vertices, int w, int h, vec3 pos){
    float current_x, current_y;
    int res_x = 16;       //16x16 vertices
    int res_y = 16;       //16x16 vertices
    float x_step = (float) w / (float) res_x;
    float y_step = (float) h / (float) res_y;

    int array_index;
    vec3 vert;

    for(unsigned short i = 0; i < res_x; i ++){
        current_x = i * x_step;

        for(unsigned short j = 0; j < res_y; j ++) {

            array_index = (i * res_y + j);
            current_y = j*y_step;
            
            vert = vec3(current_x, current_y, 0);

            indexed_vertices.resize(indexed_vertices.size() + 1);
            indexed_vertices[array_index] = vert;

        }
    }
}

void generatePlan(std::vector<std::vector<short unsigned int>> &triangles, std::vector<glm::vec3> &indexed_vertices, int w, int h, vec3 pos) {
    int res_x = 16;       //16x16 vertices
    int res_y = 16;       //16x16 vertices

    int array_index;
    int tr_index = 0;

    generateVertices(indexed_vertices, w, h, pos);
    
    for(unsigned short i = 0; i < res_x - 1; i ++){
        for(unsigned short j = 0; j < res_y - 1; j ++) {

            array_index = (i * res_y + j);
            
            triangles.resize(triangles.size() + 2);
            triangles[tr_index].resize(3);
            triangles[tr_index][0] = array_index; 
            triangles[tr_index][1] = array_index + 1; 
            triangles[tr_index][2] = array_index + res_y; 
            
            triangles[tr_index + 1].resize(3);
            triangles[tr_index + 1][0] = array_index + res_y;
            triangles[tr_index + 1][1] = array_index + 1;
            triangles[tr_index + 1][2] = array_index + 1 + res_y;

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
    window = glfwCreateWindow( 1024, 768, "TP1 - GLFW", NULL, NULL);
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

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms
    GLuint mvp_handle = glGetUniformLocation(programID, "mvp");

    /****************************************/
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;

    //Chargement du fichier de maillage
    std::string filename("chair.off");
    loadOFF(filename, indexed_vertices, indices, triangles );

    // // Load it into a VBO
    // GLuint vertexbuffer;
    // glGenBuffers(1, &vertexbuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // // Generate a buffer for the indices as well
    // GLuint elementbuffer;
    // glGenBuffers(1, &elementbuffer);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");



    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    //PLANE....
    std::vector<std::vector<unsigned short> > triangles_arr;
    std::vector<glm::vec3> indexed_vert;
    std::vector<unsigned short> soup;

    generatePlan(triangles_arr, indexed_vert, 4, 4, vec3(0, 0, 0));
    soup.resize(3 * triangles_arr.size());
    int k = 0;
    for(int i = 0; i < triangles_arr.size(); i ++){
        soup[k] = triangles_arr[i][0]; 
        soup[k + 1] = triangles_arr[i][1]; 
        soup[k + 2] = triangles_arr[i][2]; 
        k += 3;
    }

    // Load it into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vert.size() * sizeof(glm::vec3), &indexed_vert[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, soup.size() * sizeof(unsigned short), &soup[0] , GL_STATIC_DRAW);


    std::cout<<triangles_arr.size()<<std::endl;
    std::cout<<soup.size()<<std::endl;

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

        // Use our shader
        glUseProgram(programID);


        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change
        glm::mat4 Model = glm::mat4(1.0f);
        
        Model =  glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
        
        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        glm::mat4 View = glm::lookAt( camera_position, camera_target + camera_position, camera_up);
        
        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);
        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms
        Model =  glm::translate(Model, vec3(-0.5, 0, 0));

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

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        // //2nd chair
        // glm::mat4 Model2 =  glm::rotate(Model, glm::radians(180.0f), glm::vec3(0, 1, 0));
        // Model2 = glm::translate(Model2, glm::vec3(-1, 0, 0));
        // mvp = Projection * View * Model2;
        // glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
        // glDrawElements(
        //             GL_TRIANGLES,      // mode
        //             indices.size(),    // count
        //             GL_UNSIGNED_SHORT,   // type
        //             (void*)0           // element array buffer offset
        //             );

        // //3rd chair
        // glm::mat4 Model3 = glm::mat4(1.0f);
        // Model3 = glm::translate(Model3, glm::vec3(0, 1, 0.));
        // Model3 = glm::rotate(Model3, angle, glm::vec3(0, 0, 1));
        // Model3 = glm::translate(Model3, glm::vec3(0, -0.5, 0.));

        // mvp = Projection * View * Model3;
        // glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
        // glDrawElements(
        //             GL_TRIANGLES,      // mode
        //             indices.size(),    // count
        //             GL_UNSIGNED_SHORT,   // type
        //             (void*)0           // element array buffer offset
        //             );

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
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
