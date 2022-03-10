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
float zoom = 1.;
vec3 orbital_axis;

//res
int resolution = 16;

bool updateMesh = false;
bool orbital = false;

float transl =0;

void initBuffers(std::vector<unsigned short> indices, std::vector<glm::vec3> vertices, std::vector<glm::vec2> uvs, GLuint& vertexbuffer, GLuint& elementbuffer, GLuint& uvbuffer){
    // Generate a buffer for UVs
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    
    // Load vertices into a VBO
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
}


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
    //ROOT plane
    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> vert_uv;
    //CHILD plane
    std::vector<unsigned short> indices_ch; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles_ch;
    std::vector<glm::vec3> indexed_vertices_ch;
    std::vector<glm::vec2> vert_uv_ch;

    // HEIGHTMAPS/TEXTURES
   
    int sun_loc = glGetUniformLocation(programID, "sun_texture");
    int earth_loc = glGetUniformLocation(programID, "earth_texture");
    int moon_loc = glGetUniformLocation(programID, "snowRock_texture");

    glUseProgram(programID);
    GLuint tex0 = loadBMP_custom("../texture/sun.bmp", 1, sun_loc);
    GLuint tex1 = loadBMP_custom("../texture/earth.bmp", 2, earth_loc);
    GLuint tex2 = loadBMP_custom("../texture/capy_ppm.bmp", 3, moon_loc);

    loadOFF("../OFF/sphere.off", indexed_vertices, indices, triangles);
    compute_sphere_uv(indexed_vertices, vert_uv);
    
    loadOFF("../OFF/sphere.off", indexed_vertices_ch, indices_ch, triangles_ch);
    compute_sphere_uv(indexed_vertices_ch, vert_uv_ch);

    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint vertexbuffer;
    GLuint elementbuffer;
    GLuint uvbuffer;
    GLuint vertexbuffer_ch, elementbuffer_ch, uvbuffer_ch;

    //ROOT ENTITY (SUN)
    Entity root;
    Transform tr;
    Mesh sun(indexed_vertices, triangles, vert_uv);
    initBuffers(sun.getIndices(), indexed_vertices, vert_uv, vertexbuffer, elementbuffer, uvbuffer);
    sun.buffers.element = elementbuffer;
    sun.buffers.vertex = vertexbuffer;
    sun.buffers.uv = uvbuffer;       
    root.addMesh(sun);
    root.addTransformation(tr);

    //CHILD_1 ENTITY (EARTH)
    Entity ch_1;
    Transform tr_1;
    Mesh earth(indexed_vertices_ch, triangles_ch, vert_uv_ch);
    initBuffers(earth.getIndices(), indexed_vertices_ch, vert_uv_ch, vertexbuffer_ch, elementbuffer_ch, uvbuffer_ch);
    earth.buffers.element = elementbuffer_ch;
    earth.buffers.vertex = vertexbuffer_ch;
    earth.buffers.uv = uvbuffer_ch;
    ch_1.addMesh(earth);
    ch_1.addTransformation(tr_1); 
    root.addChild(ch_1);
    

    // ORIGINAL TRANSFORMATIONS
    root.transform.setLocalPosition(vec3(0, 0, 0));
    root.updateSelfAndChild();
    ch_1.transform.setLocalPosition(vec3(-2, 0, 0));
    ch_1.transform.scale = vec3(0.5, 0.5, 0.5);
    ch_1.updateSelfAndChild();
    vec3 pos = ch_1.transform.pos;
    

    if(ch_1.parent == nullptr)
        std::cout<<"NO PARENT"<<std::endl;
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

        // Use our shader
        glUseProgram(programID);

        
        // Model matrix : an identity matrix (model will be at the origin) then change
        // glm::mat4 Model = glm::mat4(1.0f);

        glm::mat4 View;
        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        View = glm::lookAt( camera_position, camera_target + camera_position, camera_up); 
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 4 / (float) 3, 0.1f, 100.0f);
        
        //PROJECTION et VIEW (STATIC PART)(caméra)
        glUniformMatrix4fv(projection_handle, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(view_handle, 1, GL_FALSE, &View[0][0]);


        //VARIABLE PART
        glm::mat4 Model = root.transform.modelMatrix;
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_tex_handle, 0);
        sun.loadToGpu(programID);
        sun.draw();   // DESSIN DU PREMIER MESH


        ch_1.transform.setSelfRotate_Y(1.5*angle);
        ch_1.transform.rot.y = 2.f * angle;
        ch_1.updateSelfAndChild();
    

        Model = ch_1.transform.modelMatrix;
        glUniformMatrix4fv(model_handle, 1, GL_FALSE, &Model[0][0]);
        glUniform1i(using_tex_handle, 1);
        earth.loadToGpu(programID);
        earth.draw();    // DESSIN DU 2nd MESH

        angle += rota_speed;
        /****************************************/

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
    glDeleteBuffers(1, &vertexbuffer_ch);
    glDeleteBuffers(1, &elementbuffer_ch);
    glDeleteBuffers(1, &uvbuffer_ch);
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
    //TODO add translations
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        rota_speed += 0.01;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        rota_speed = (rota_speed - 0.01) < 0?rota_speed: rota_speed- 0.01;
    }


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        if(orbital){
            zoom += 0.1;
            orbital_axis = vec3(1, 0, 0);
            // cos(z) sin(y)
            // camera_position += vec3(0., sin(cameraSpeed), cos(cameraSpeed)) + camera_target;
            
        }else {

            camera_position += cameraSpeed * camera_target;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        if(orbital){
            zoom -= 0.1;
            orbital_axis = vec3(1, 0, 0);

        }else{
            camera_position -= cameraSpeed * camera_target;

        }

    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        if(orbital){
            zoom += 0.1;
            orbital_axis = vec3(0., 1, 0);

        }else{

            camera_position -= glm::vec3(cameraSpeed, 0, 0) ;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        if(orbital){
            zoom -= 0.1;
            orbital_axis = vec3(0, 1, 0);
        }else{
            camera_position += glm::vec3(cameraSpeed, 0, 0);

        }
    }
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        angle += rota_speed;
        
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
