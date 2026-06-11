#include "GeoMesh.h"

void GeoMesh::generate(std::vector<float>& vertexData, std::vector<unsigned int>& indices)
{
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);

  sizeOfIndices = indices.size();
}

void GeoMesh::bind()
{
  glBindVertexArray(vao);
}

void GeoMesh::unbind()
{
  glBindVertexArray(0);
}

int GeoMesh::size()
{
  return sizeOfIndices;
}

GeoMesh::GeoMesh()
{}

GeoMesh::GeoMesh(int type, int blocksWidth, float spacing)
{
  create(type, blocksWidth, spacing);
}

GeoMesh::~GeoMesh()
{}

void GeoMesh::create(int type, int blocksWidth, float spacing)
{
  int radius = (blocksWidth/2);
  int vdSize = 0, indSize = 0;

  if(type == GRID_FULL)
  {
    vdSize = (2 * radius + 1)*(2 * radius + 1); // L * W
    vdSize = 2 * vdSize; // x and z
    std::vector<float> vertexData(vdSize);
    for(float x=-radius; x<=radius; x++)
    {
      for(float z=-radius; z<=radius; z++)
      {
        int index = (x+radius)*(2*radius+1) + (z+radius);
        vertexData[2*index + 0] = spacing * x;
        vertexData[2*index + 1] = spacing * z;
      }
    }
    
    indSize = blocksWidth * blocksWidth; // L * W
    indSize = 6 * indSize; // 6 per block
    std::vector<unsigned int> indices(indSize);
    for(int i=0; i<blocksWidth; i++)
    {
      for(int j=0; j<blocksWidth; j++)
      {
        int a = i*(blocksWidth + 1) + j;
        int b = a + 1;
        int c = (i+1)*(blocksWidth + 1) + j;
        int d = c + 1;
        
        int index = i*blocksWidth + j;
        indices[6*index + 0] = a;
        indices[6*index + 1] = c;
        indices[6*index + 2] = b;
        indices[6*index + 3] = b;
        indices[6*index + 4] = c;
        indices[6*index + 5] = d;
      }
    }
    generate(vertexData, indices);
  }
  else if(type == GRID_HOLLOW)
  {
    int hollowRadius = (blocksWidth/2 + 1)/2 - 1;
    vdSize = ((2*radius+1)*(2*radius+1)) - ((2*hollowRadius+1)*(2*hollowRadius+1)); // L * W - (l_hollow * w_hollow)
    vdSize = 2 * vdSize; // x and z
    std::vector<float> vertexData(vdSize);
    int index = 0;
    for(float x=-radius; x<=radius; x++)
    {
      for(float z=-radius; z<=radius; z++)
      {
        if(x >= -hollowRadius && x <= hollowRadius
            && z >= -hollowRadius && z <= hollowRadius)
              continue; // skip
        vertexData[2*index + 0] = spacing * x;
        vertexData[2*index + 1] = spacing * z;
        index++;
      }
    }
    
    indSize = blocksWidth*blocksWidth - ( (blocksWidth/2 + 1)*(blocksWidth/2 +1) ); // L * W - (l_hollow * w_hollow)
    indSize = 6 * indSize; // 6 per block
    std::vector<unsigned int> indices(indSize);
    int cut = ( (blocksWidth) - (blocksWidth/2+1) )/2;
    int cutI = 0, cutJ = 0;

    int cutStart = (radius - hollowRadius) * (blocksWidth + 1)  + (radius - hollowRadius);
    int cutMax = (2*hollowRadius + 1)*(2*hollowRadius + 1); 
    index = 0;
    for(int i=0; i<blocksWidth; i++)
    {
      for(int j=0; j<blocksWidth; j++)
      {
        if(i >= cut && i < blocksWidth - cut 
            && j >= cut && j < blocksWidth - cut)
        {
          continue;
        }


        int a = i*(blocksWidth + 1) + j;
        int b = a + 1;
        int c = (i+1)*(blocksWidth + 1) + j;
        int d = c + 1;


        int aCut = ((a-cutStart)/(blocksWidth+1) + 1) * (2*hollowRadius + 1);
        aCut = aCut < cutMax ? aCut : cutMax;
        int bCut = ((b-cutStart)/(blocksWidth+1) + 1) * (2*hollowRadius + 1);
        bCut = bCut < cutMax ? bCut : cutMax;
        int cCut = ((c-cutStart)/(blocksWidth+1) + 1) * (2*hollowRadius + 1);
        cCut = cCut < cutMax ? cCut : cutMax;
        int dCut = ((d-cutStart)/(blocksWidth+1) + 1) * (2*hollowRadius + 1);
        dCut = dCut < cutMax ? dCut : cutMax;

        a = a - cutStart >= 0 ? a - aCut : a;
        b = b - cutStart >= 0 ? b - bCut : b;
        c = c - cutStart >= 0 ? c - cCut : c;
        d = d - cutStart >= 0 ? d - dCut : d;
        

        indices[6*index + 0] = a;
        indices[6*index + 1] = c;
        indices[6*index + 2] = b;
        indices[6*index + 3] = b;
        indices[6*index + 4] = c;
        indices[6*index + 5] = d;
        index++;
      }
    }
    generate(vertexData, indices);
  }
  else if(type == STRIP)
  {
    int stripBlocksWidth = ((blocksWidth/2) + 1);
    vdSize = (stripBlocksWidth + 1) * 2;
    vdSize += (stripBlocksWidth - 1) * 2;
    vdSize = 2 * vdSize;
    indSize = 2*stripBlocksWidth - 1; 
    indSize = 6 * indSize;

    std::vector<float> vertexData;
    vertexData.reserve(vdSize);
    std::vector<unsigned int> indices(indSize);
    
    int stride = 0;
    float theZ = 0.0f;
    for(int i=0; i<=stripBlocksWidth; i++)
    {
      //vertexData[stride + 2*i + 0] = (i - 0.5f) * spacing;
      //vertexData[stride + 2*i + 1] = (theZ - 0.5f) * spacing;
      vertexData.push_back( (i - 0.5f) * spacing );
      vertexData.push_back( (theZ - 0.5f) * spacing );
    }

    stride += 2*(stripBlocksWidth+1);
    theZ = 1.0f;
    for(int i=0; i<=stripBlocksWidth; i++)
    {
      //vertexData[stride + 2*i + 0] = (i - 0.5f) * spacing;
      //vertexData[stride + 2*i + 1] = (theZ - 0.5f) * spacing;
      vertexData.push_back( (i - 0.5f) * spacing );
      vertexData.push_back( (theZ - 0.5f) * spacing );
    }
    stride += 2*(stripBlocksWidth+1);
    for(int i=0; i<stripBlocksWidth; i++)
    {
      //vertexData[stride + 4*i + 0] = (0 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 1] = (i+2 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 2] = (1 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 3] = (i+2 - 0.5f) * spacing;

      vertexData.push_back( (0 - 0.5f) * spacing );
      vertexData.push_back( (i+2 - 0.5f) * spacing );
      vertexData.push_back( (1 - 0.5f) * spacing );
      vertexData.push_back( (i+2 - 0.5f) * spacing );
    }

    // horizontal first
    for(int i=0; i<stripBlocksWidth; i++)
    {
      int a = i;
      int b = i+1;
      int c = i + (stripBlocksWidth + 1);
      int d = i + (stripBlocksWidth + 1) + 1;
      indices[6*i + 0] = a;
      indices[6*i + 1] = c;
      indices[6*i + 2] = b;
      indices[6*i + 3] = b;
      indices[6*i + 4] = c;
      indices[6*i + 5] = d;
    }
    stride = stripBlocksWidth*6;
    
    int a = (stripBlocksWidth + 1);
    int b = (stripBlocksWidth + 1) + 1;
    int c = 2*(stripBlocksWidth + 1);
    int d = 2*(stripBlocksWidth + 1) + 1;

    indices[stride + 0] = a;
    indices[stride + 1] = c;
    indices[stride + 2] = b;
    indices[stride + 3] = b;
    indices[stride + 4] = c;
    indices[stride + 5] = d;

    stride += 6;

    // vertical next
    for(int i=0; i<stripBlocksWidth-2; i++)
    {
      int a = 2*(stripBlocksWidth + 1) + 2*i;
      int b = 2*(stripBlocksWidth + 1) + 2*i + 1;
      int c = 2*(stripBlocksWidth + 1) + 2*i + 2;
      int d = 2*(stripBlocksWidth + 1) + 2*i + 3;
      indices[stride + 6*i + 0] = a;
      indices[stride + 6*i + 1] = c;
      indices[stride + 6*i + 2] = b;
      indices[stride + 6*i + 3] = b;
      indices[stride + 6*i + 4] = c;
      indices[stride + 6*i + 5] = d;
    }

    generate(vertexData, indices);
  }
  else if(type == STRIP_ALT)
  {
    int stripBlocksWidth = ((blocksWidth/2) + 1);
    vdSize = (stripBlocksWidth + 1) * 2;
    vdSize += (stripBlocksWidth - 1) * 2;
    vdSize = 2 * vdSize;
    indSize = 2*stripBlocksWidth - 1; 
    indSize = 6 * indSize;

    std::vector<float> vertexData;
    vertexData.reserve(vdSize);
    std::vector<unsigned int> indices(indSize);
    
    int stride = 0;
    float theZ = 0.0f;
    for(int i=0; i<=stripBlocksWidth; i++)
    {
      //vertexData[stride + 2*i + 0] = (i - 0.5f) * spacing;
      //vertexData[stride + 2*i + 1] = (theZ - 0.5f) * spacing;
      vertexData.push_back((i - 0.5f) * spacing);
      vertexData.push_back((theZ - 0.5f) * spacing);
    }

    stride += 2*(stripBlocksWidth+1);
    theZ = 1.0f;
    for(int i=0; i<=stripBlocksWidth; i++)
    {
      //vertexData[stride + 2*i + 0] = (i - 0.5f) * spacing;
      //vertexData[stride + 2*i + 1] = (theZ - 0.5f) * spacing;
      vertexData.push_back((i - 0.5f) * spacing);
      vertexData.push_back((theZ - 0.5f) * spacing);
    }
    stride += 2*(stripBlocksWidth+1);
    for(int i=0; i<stripBlocksWidth; i++)
    {
      //vertexData[stride + 4*i + 0] = (0 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 1] = (i+2 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 2] = (1 - 0.5f) * spacing;
      //vertexData[stride + 4*i + 3] = (i+2 - 0.5f) * spacing;
      vertexData.push_back( (0 - 0.5f) * spacing);
      vertexData.push_back( (i+2 - 0.5f) * spacing);
      vertexData.push_back( (1 - 0.5f) * spacing);
      vertexData.push_back( (i+2 - 0.5f) * spacing);
    }

    // horizontal first
    for(int i=0; i<stripBlocksWidth; i++)
    {
      int a = i;
      int b = i+1;
      int c = i + (stripBlocksWidth + 1);
      int d = i + (stripBlocksWidth + 1) + 1;
      indices[6*i + 0] = a;
      indices[6*i + 1] = c;
      indices[6*i + 2] = d;
      indices[6*i + 3] = b;
      indices[6*i + 4] = a;
      indices[6*i + 5] = d;
    }
    stride = stripBlocksWidth*6;
    
    int a = (stripBlocksWidth + 1);
    int b = (stripBlocksWidth + 1) + 1;
    int c = 2*(stripBlocksWidth + 1);
    int d = 2*(stripBlocksWidth + 1) + 1;

    indices[stride + 0] = a;
    indices[stride + 1] = c;
    indices[stride + 2] = d;
    indices[stride + 3] = b;
    indices[stride + 4] = a;
    indices[stride + 5] = d;

    stride += 6;

    // vertical next
    for(int i=0; i<stripBlocksWidth-2; i++)
    {
      int a = 2*(stripBlocksWidth + 1) + 2*i;
      int b = 2*(stripBlocksWidth + 1) + 2*i + 1;
      int c = 2*(stripBlocksWidth + 1) + 2*i + 2;
      int d = 2*(stripBlocksWidth + 1) + 2*i + 3;
      indices[stride + 6*i + 0] = a;
      indices[stride + 6*i + 1] = c;
      indices[stride + 6*i + 2] = d;
      indices[stride + 6*i + 3] = b;
      indices[stride + 6*i + 4] = a;
      indices[stride + 6*i + 5] = d;
    }

    generate(vertexData, indices);
  }
}
