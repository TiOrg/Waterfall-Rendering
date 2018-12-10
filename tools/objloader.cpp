#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "objloader.hpp"

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices
)
{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}
        else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[4], trash;
            
//            int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0],  &vertexIndex[1],  &vertexIndex[2]);
//            if (matches != 3){
//                printf("File can't be read by parser :-(\n");
//                fclose(file);
//                return false;
//            }
//            vertexIndices.push_back(vertexIndex[0]);
//            vertexIndices.push_back(vertexIndex[1]);
//            vertexIndices.push_back(vertexIndex[2]);

            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &trash, &trash, &vertexIndex[1], &trash, &trash, &vertexIndex[2], &trash, &trash, &vertexIndex[3], &trash, &trash );
            if (matches != 12)
            {
                printf("File can't be read by parser :-(\n");
                fclose(file);
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);

//            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &trash, &trash, &vertexIndex[1], &trash, &trash, &vertexIndex[2], &trash, &trash, &vertexIndex[3], &trash, &trash );
//            if( matches == 12)
//            {
//                vertexIndices.push_back(vertexIndex[0]);
//                vertexIndices.push_back(vertexIndex[1]);
//                vertexIndices.push_back(vertexIndex[2]);
//
//                vertexIndices.push_back(vertexIndex[1]);
//                vertexIndices.push_back(vertexIndex[2]);
//                vertexIndices.push_back(vertexIndex[3]);
//
//            }
//            else if(matches == 9)
//            {
//                vertexIndices.push_back(vertexIndex[0]);
//                vertexIndices.push_back(vertexIndex[1]);
//                vertexIndices.push_back(vertexIndex[2]);
//            }
//            else
//            {
//                printf("File can't be read by parser :-(\n");
//                fclose(file);
//                return false;
//            }

//            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &trash, &trash, &vertexIndex[1], &trash, &trash, &vertexIndex[2], &trash, &trash );
//            if (matches != 9)
//            {
//                printf("File can't be read by parser :-(\n");
//                fclose(file);
//                return false;
//            }
//            vertexIndices.push_back(vertexIndex[0]);
//            vertexIndices.push_back(vertexIndex[1]);
//            vertexIndices.push_back(vertexIndex[2]);

            
		}
        else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
	
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);

	}
	fclose(file);
	return true;
}

