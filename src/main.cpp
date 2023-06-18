#include <iostream>
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
#include "mesh.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util.h"
#include "camera.h"


#define _USE_MATH_DEFINES
typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;
static const int SRC_WIDTH = 1200;
static const int SRC_HEIGHT = 800;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SRC_WIDTH / 2.0f;
float lastY = SRC_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


auto fpsCount = 0;
auto m_lastFPSDisplayTimestamp = glfwGetTime();

glm::vec3 lightPos(0.4f, 1.0f, 1.2f);




int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "bunny.obj";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Laplacian Smoothing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

#if(WIN32)
    Shader faceShader("../../../src/shader.vs", "../../../src/shader.fs");
    Shader edgeShader("../../../src/edge_shader.vs", "../../../src/edge_shader.fs");
#else   
    Shader faceShader("../src/shader.vs", "../src/shader.fs");
    Shader edgeShader("../src/edge_shader.vs", "../src/edge_shader.fs");
#endif

    MyMesh  mesh;
    if ( ! OpenMesh::IO::read_mesh(mesh,path))
    {
        std::cerr << "Error loading mesh from file " << path << std::endl;
    }

    mesh.request_face_normals();  // Request vertex normals
    mesh.update_face_normals();
    mesh.request_vertex_normals();  // Request vertex normals
    mesh.update_vertex_normals();
    auto edit = OpenMesh::VProp<float>(mesh,"edit");

    Mesh obj(mesh);
    int iteration = 0;

    auto vertex_iter = mesh.vertices().begin();

    while (!glfwWindowShouldClose(window))
    {
        auto m_currentFrameTimestamp = glfwGetTime();

        if((m_currentFrameTimestamp - m_lastFPSDisplayTimestamp) >= 0.001){
            MyMesh::Scalar valence(0.0);

            MyMesh::Point  cog = MyMesh::Point(0.0, 0.0, 0.0);
            for (auto vv_it=mesh.vv_iter(*vertex_iter); vv_it.is_valid(); ++vv_it)
            {
                cog += mesh.point( *vv_it );
                ++valence;
            }
            cog /= valence;

            if (!mesh.is_boundary(*vertex_iter))
                mesh.set_point( *vertex_iter, cog);

            edit[*vertex_iter] += 0.2f;
            //auto pv = std::prev(vertex_iter, 1);
            //edit[*pv] = 0.0f;
             obj = Mesh(mesh);
            m_lastFPSDisplayTimestamp = m_currentFrameTimestamp;
            if(vertex_iter != mesh.vertices_end())
                vertex_iter++;
            else {
                vertex_iter = mesh.vertices_begin();
                std::cout << "Iteration: " << iteration << "\n";
                ++iteration;
            }

            m_lastFPSDisplayTimestamp = m_currentFrameTimestamp;
        }

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window,camera,deltaTime);

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        faceShader.use();
        faceShader.setVec3("lightPos", lightPos);
        faceShader.setVec3("viewPos", camera.Position);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        faceShader.setMat4("projection", projection);
        faceShader.setMat4("view", view);


        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,  -1.0f, 0.5f));
        model = glm::rotate(model, -0.2f, glm::vec3(0.0f, 1.0f, 0.0f)); // where x, y, z is axis of rotation (e.g. 0 1 0)
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        faceShader.setMat4("model", model);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        glDepthFunc(GL_LESS);
        obj.draw_faces(faceShader);

        // render the mesh skeleton
        edgeShader.use();
        edgeShader.setMat4("projection", projection);
        edgeShader.setMat4("view", view);
        edgeShader.setMat4("model", model);
        glDisable(GL_POLYGON_OFFSET_FILL);

        glDepthFunc(GL_LEQUAL);
        obj.draw_edges(edgeShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}



