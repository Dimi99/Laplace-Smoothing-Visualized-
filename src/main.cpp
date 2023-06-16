#include <iostream>
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
#include "mesh.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util.h"
#include "camera.h"

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




int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "bunny.obj";

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);



    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader faceShader("../src/shader.vs", "../src/shader.fs");
    Shader edgeShader("../src/edge_shader.vs", "../src/edge_shader.fs");


    MyMesh  mesh;
    if ( ! OpenMesh::IO::read_mesh(mesh,path))
    {
        std::cerr << "Error loading mesh from file " << path << std::endl;
        return 1;
    }
    auto points = OpenMesh::getPointsProperty(mesh);
    //auto faces = OpenMesh::makePropertyManagerFromExistingOrNew(mesh,"face",)

    std::vector<Vertex> vertices;
    std::vector<unsigned int> edges;
    std::vector<unsigned int> indices;
    mesh.request_face_normals();  // Request vertex normals
    mesh.update_face_normals();
    mesh.request_vertex_normals();  // Request vertex normals
    mesh.update_vertex_normals();


    for(auto vh: mesh.vertices()){
        Vertex vertex{};
        glm::vec3 vector;
        // positions
        vector.x = points(vh)[0];
        vector.y = points(vh)[1];
        vector.z = points(vh)[2];
        vertex.Position = vector;
        vertex.Normal = glm::vec3(mesh.normal(vh)[0],mesh.normal(vh)[1],mesh.normal(vh)[2]);
        vertices.push_back(vertex);
    }

    indices.reserve(mesh.n_faces() * 3);
    for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it)
    {
        for (MyMesh::FaceVertexIter vv_it=mesh.fv_iter(*f_it); vv_it.is_valid(); ++vv_it)
            indices.push_back(vv_it->idx());
    }
    edges.reserve(mesh.n_edges()*3);
    // Traverse all edges in the mesh
    for (MyMesh ::EdgeIter edge_it = mesh.edges_begin(); edge_it != mesh.edges_end(); ++edge_it)
    {
        // Get one of the halfedges of the current edge
        MyMesh::HalfedgeHandle halfedge_handle = mesh.halfedge_handle(*edge_it, 0);

        // Get the vertex indices of the halfedge endpoints
        unsigned vertexIndex1 = mesh.to_vertex_handle(halfedge_handle).idx();
        //unsigned vertexIndex2 = mesh.to_vertex_handle(mesh.next_halfedge_handle(halfedge_handle)).idx();
        unsigned vertexIndex2 = mesh.from_vertex_handle(halfedge_handle).idx(); 

        edges.push_back(vertexIndex1);
        edges.push_back(vertexIndex2);
    }


    Mesh obj(vertices,indices,edges);
    int i = 0;
    int j = 0;
    while (!glfwWindowShouldClose(window))
    {
        auto m_currentFrameTimestamp = glfwGetTime();

        if ((m_currentFrameTimestamp - m_lastFPSDisplayTimestamp) >= 1) {
            obj.update_buffer();
            obj.vertices.at(i).edit = 0.0f;
            obj.vertices.at(++i).edit = 1.0f;
            m_lastFPSDisplayTimestamp = m_currentFrameTimestamp;

        }




        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window,camera,deltaTime);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        faceShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        faceShader.setMat4("projection", projection);
        faceShader.setMat4("view", view);



        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,  -2.5f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
        faceShader.setMat4("model", model);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0); ///< may need adjustment for your use case
        glDepthFunc(GL_LESS);
        obj.draw_faces(faceShader);


        edgeShader.use();
        edgeShader.setMat4("projection", projection);
        edgeShader.setMat4("view", view);
        edgeShader.setMat4("model", model);
        glDisable(GL_POLYGON_OFFSET_FILL);

        glDepthFunc(GL_LEQUAL);
        obj.draw_edges(edgeShader);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}



