#ifndef MESH_H
#define MESH_H

#include "glad/glad.h"
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;




#if(WIN32)
#include "../libs/glm/glm/glm.hpp"
#include "../libs/glm/glm/gtc/matrix_transform.hpp"


#else
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include "shader.h"
#include <string>
#include <utility>
#include <vector>

#define MAX_BONE_INFLUENCE 200

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    float edit = 0.0f;

};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> edges;
    unsigned int VAO{};
    unsigned int VAO_EDGE{};
    Mesh(MyMesh& mesh)
    {
        auto points = OpenMesh::getPointsProperty(mesh);
        auto edits = OpenMesh::VProp<float>(mesh, "edit");
        //auto faces = OpenMesh::makePropertyManagerFromExistingOrNew(mesh,"face",)


        for(auto &vh: mesh.vertices()){
            Vertex vertex{};
            glm::vec3 vector;
            // positions
            vector.x = points(vh)[0];
            vector.y = points(vh)[1];
            vector.z = points(vh)[2];
            vertex.Position = vector;
            vertex.Normal = glm::vec3(mesh.normal(vh)[0],mesh.normal(vh)[1],mesh.normal(vh)[2]);
            vertex.edit = edits[vh];
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
        for (MyMesh::EdgeIter edge_it = mesh.edges_begin(); edge_it != mesh.edges_end(); ++edge_it)
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

        setupMesh();
    }

    ~Mesh(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteVertexArrays(1, &VAO_EDGE);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &VBO_EDGE);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &EBO_EDGE);

    }

    Mesh& operator=(Mesh const& other)
    {
        if(this == &other)
            return *this;
        vertices = other.vertices;
        edges = other.edges;
        indices = other.indices;


        glDeleteVertexArrays(1, &VAO);
        glDeleteVertexArrays(1, &VAO_EDGE);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &VBO_EDGE);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &EBO_EDGE);

        this->VAO = other.VAO;
        this->VAO_EDGE = other.VAO_EDGE;
        this->VBO = other.VBO;
        this->VBO_EDGE = other.VBO_EDGE;
        this->EBO = other.EBO;
        this ->EBO_EDGE = other.EBO_EDGE;
        setupMesh();
       return *this;
    }



    void draw_faces(Shader &shader)
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }
    void draw_edges(Shader &shader)
    {
        glBindVertexArray(VAO_EDGE);
        glDrawElements(GL_LINES, static_cast<unsigned int>(edges.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

    void update_buffer(){
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, edit));

        glBindVertexArray(VAO_EDGE);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_EDGE);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_EDGE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, edges.size() * sizeof(unsigned int), &edges[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, edit));
    }

protected:
    unsigned int VBO{}, EBO{};
    unsigned int VBO_EDGE{}, EBO_EDGE{};

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, edit));

        glGenVertexArrays(1, &VAO_EDGE);
        glGenBuffers(1, &VBO_EDGE);
        glGenBuffers(1, &EBO_EDGE);

        glBindVertexArray(VAO_EDGE);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO_EDGE);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_EDGE);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, edges.size() * sizeof(unsigned int), &edges[0], GL_STATIC_DRAW);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, edit));

    }

};

#endif