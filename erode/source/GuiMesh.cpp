#include "GuiMesh.h"

void GuiMesh::generate(std::vector<float>& vertexData, std::vector<unsigned int>& indices)
{
  glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	sizeOfIndices = indices.size();

}

GuiMesh::GuiMesh()
{
  std::vector<float> vertexData = {
    // vertex   // texture 
    -1, 1,      0, 1,     
    1, 1,       1, 1,
    -1, -1,     0, 0,
    1, -1,      1, 0,
  };

  std::vector<unsigned int> indices = {
    0,  1,  2,  2,  1,  3  
  };

  generate(vertexData, indices);
}

GuiMesh::~GuiMesh()
{
}

void GuiMesh::bind()
{
  glBindVertexArray(vao);
}

void GuiMesh::unbind()
{
  glBindVertexArray(0);
}

int GuiMesh::size()
{
  return sizeOfIndices;
}
