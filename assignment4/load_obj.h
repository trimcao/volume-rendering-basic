#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>


struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

void load_obj(std::string obj_path, std::vector<Vertex>&vertices)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;

    std::ifstream ifs;
    try
    {
        ifs.open(obj_path);
        std::string one_line;
        while (getline(ifs, one_line))
        {
            std::stringstream ss(one_line);
            std::string type;
            ss >> type;
            if (type == "v")
            {
                glm::vec3 vert_pos;
                ss >> vert_pos[0] >> vert_pos[1] >> vert_pos[2];
                positions.push_back(vert_pos);
            }
            else if (type == "vt")
            {
                glm::vec2 tex_coord;
                ss >> tex_coord[0] >> tex_coord[1];
                // flip the y coordinate of the texture image
                tex_coord = glm::vec2(tex_coord.x, 1.0 - tex_coord.y);
                tex_coords.push_back(tex_coord);
            }
            else if (type == "vn")
            {
                glm::vec3 vert_norm;
                ss >> vert_norm[0] >> vert_norm[1] >> vert_norm[2];
                normals.push_back(vert_norm);
            }
            else if (type == "f")
            {
                std::string s_vertex_0, s_vertex_1, s_vertex_2;
                ss >> s_vertex_0 >> s_vertex_1 >> s_vertex_2;
                int pos_idx, tex_idx, norm_idx;
                sscanf(s_vertex_0.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
                // We have to use index -1 because the obj index starts at 1
                Vertex vertex_0;
                vertex_0.Position = positions[pos_idx-1];
                vertex_0.TexCoords = tex_coords[tex_idx-1];
                vertex_0.Normal = normals[norm_idx-1];

                sscanf(s_vertex_1.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
                // We have to use index -1 because the obj index starts at 1
                Vertex vertex_1;
                vertex_1.Position = positions[pos_idx-1];
                vertex_1.TexCoords = tex_coords[tex_idx-1];
                vertex_1.Normal = normals[norm_idx-1];

                sscanf(s_vertex_2.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
                // We have to use index -1 because the obj index starts at 1
                Vertex vertex_2;
                vertex_2.Position = positions[pos_idx-1];
                vertex_2.TexCoords = tex_coords[tex_idx-1];
                vertex_2.Normal = normals[norm_idx-1];

                vertices.push_back(vertex_0);
                vertices.push_back(vertex_1);
                vertices.push_back(vertex_2);
            }
        }
    }
    catch (const std::exception&) {
        std::cout << "Error: Obj file cannot be read\n";
    }
}

// void load_obj(std::string obj_path, std::vector<Vertex>&vertices);
