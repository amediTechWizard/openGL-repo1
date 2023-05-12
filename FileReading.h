#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <glm/glm.hpp>

class VertexData {
public:
    float x, y, z;
    float nx, ny, nz;
    float r, g, b;
    float u, v;
};

class TriData {
public:
    int v[3];
};

void readPLYFile(const std::string& fname, std::vector<VertexData>& vertices, std::vector<TriData>& faces) {
    std::ifstream file(fname);
    if (!file) {
        std::cerr << "Error: Could not open file " << fname << std::endl;
        return;
    }
    std::string line;
    int num_vertices = 0, num_faces = 0;
    bool reading_vertices = false, reading_faces = false;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "end_header") {
            reading_vertices = true;
        }
        else if (reading_vertices) {
            VertexData v;
            iss >> v.x >> v.y >> v.z >> v.nx >> v.ny >> v.nz >> v.r >> v.g >> v.b >> v.u >> v.v;
            vertices.push_back(v);
            num_vertices++;
        }
        else if (token == "element" && !reading_faces) {
            iss >> token;
            if (token == "face") {
                iss >> num_faces;
                reading_faces = true;
            }
        }
        else if (reading_faces) {
            int count, v1, v2, v3;
            iss >> count >> v1 >> v2 >> v3;
            if (count == 3) {
                TriData t;
                t.v[0] = v1;
                t.v[1] = v2;
                t.v[2] = v3;
                faces.push_back(t);
            }
        }
    }
}

int main() {
    std::vector<VertexData> vertices;
    std::vector<TriData> faces;
    readPLYFile("LinksHouse/Walls.ply", vertices, faces);
    std::cout << "Number of vertices: " << vertices.size() << std::endl;
    std::cout << "Number of faces: " << faces.size() << std::endl;
    return 0;
}
