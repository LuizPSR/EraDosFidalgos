#include "VertexArray.hpp"
#include <GL/glew.h>

VertexArray::VertexArray(const float* verts, unsigned int numVerts, const unsigned int* indices,
						 unsigned int numIndices)
: mNumVerts(numVerts)
, mNumIndices(numIndices)
, mVertexBuffer(0)
, mIndexBuffer(0)
, mVertexArray(0)
{
	glGenVertexArrays(1, &mVertexArray);
    glBindVertexArray(mVertexArray);

	constexpr unsigned int VERTEX_SIZE_FLOATS = 4;
	constexpr unsigned int VERTEX_SIZE_BYTES = VERTEX_SIZE_FLOATS * sizeof(float);
	constexpr unsigned int POS_SIZE = 2; // Number of floats for position
	constexpr unsigned int TEX_SIZE = 2; // Number of floats for texture coords

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNumVerts * VERTEX_SIZE_BYTES, verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Enable the attribute at location 0
    glVertexAttribPointer(
        0,
        POS_SIZE,
        GL_FLOAT,
        GL_FALSE,
        VERTEX_SIZE_BYTES,
        (void*)0
    );

    glEnableVertexAttribArray(1); // Enable the attribute at location 1
    glVertexAttribPointer(
        1,
        TEX_SIZE,
        GL_FLOAT,
        GL_FALSE,
        VERTEX_SIZE_BYTES,
        (void*)(POS_SIZE * sizeof(float)) // Offset after position data
    );

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexArray::~VertexArray()
{
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVertexArray);
}

void VertexArray::SetActive() const
{
	glBindVertexArray(mVertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
}
