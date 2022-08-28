/*
* housescene.c
* author: Thomas Saulnier c. 2020
*/

#define GLFW_DLL
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define PI 3.1415926f
#define d2r(d) ((d)*(PI/180))

typedef float mat4[4][4];
typedef float vec3[3];

void matCopy(const mat4 from,mat4 to)
{
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      {
        to[i][j] = from[i][j];
      }
}

void mat4Mult(const mat4 left,const mat4 right,mat4 result)
{
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      {
        float sum = 0;
        for (int k = 0; k < 4; k++)
          sum += left[i][k]*right[k][j];
        result[i][j] = sum;
      }
}

void identity(mat4 result)
{
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      {
        result[i][j] = 0;
      }
  result[0][0] = 1;
  result[1][1] = 1;
  result[2][2] = 1;
  result[3][3] = 1;
}

//test
void rockOffset(mat4 result)
{
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      {
        result[i][j] = 0;
      }
      result[0][0] = 1;
      result[1][1] = 1;
      result[2][2] = 1;
      result[3][3] = 1;
  result[1][3] = 10;
}

void scale(float x,float y,float z,mat4 result)
{
  identity(result);
  result[0][0] = x;
  result[1][1] = y;
  result[2][2] = z;
}

void translate(float deltaX,float deltaY,float deltaZ,mat4 result)
{
  identity(result);
  result[0][3] = deltaX;
  result[1][3] = deltaY;
  result[2][3] = deltaZ;
}

void rotateX(float radians,mat4 result)
{
  identity(result);
  float c = (float)cos(radians);
  float s = (float)sin(radians);
  result[1][1] = c;
  result[2][2] = c;
  result[1][2] = -s;
  result[2][1] = s;
}


void rotateY(float radians,mat4 result)
{
  identity(result);
  float c = (float)cos(radians);
  float s = (float)sin(radians);
  result[0][0] = c;
  result[2][2] = c;
  result[0][2] = -s;
  result[2][0] = s;
}


void rotateZ(float radians,mat4 result)
{
  identity(result);
  float c = (float)cos(radians);
  float s = (float)sin(radians);
  result[0][0] = c;
  result[1][1] = c;
  result[0][1] = -s;
  result[1][0] = s;
}

void perspective(float near,float far,mat4 result)
{
  identity(result);
  result[0][0] = near;
  result[1][1] = near;
  result[2][2] = (-near-far)/(far-near);
  result[2][3] = -2*near*far/(far-near);
  result[3][2] = -1;
  result[3][3] = 0;
}

void perspectivea( float fovy, float aspect, float near, float far, mat4 result) {
	float fov_rad = d2r(fovy);
	float inverse_range = 1.0f / tan( fov_rad / 2.0f );
	float sx = inverse_range / aspect;
	float sy = inverse_range;
	float sz = -( far + near ) / ( far - near );
	float pz = -( 2.0f * far * near ) / ( far - near );
    identity(result);
	result[0][0] = sx;
	result[1][1] = sy;
	result[2][2] = sz;
	result[2][3] = -1.0f;
    result[3][2] = pz;
    result[3][3] = 0;
}


void cross(const vec3 v1,const vec3 v2,vec3 product)
{
   product[0] = v1[1]*v2[2] - v1[2]*v2[1];
   product[1] = v1[2]*v2[0] - v1[0]*v2[2];
   product[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void normalize(const vec3 v,vec3 result)
{
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float sum = x*x+y*y+z*z;
    result[0] = (float)sqrt(x*x/sum);
    result[1] = (float)sqrt(y*y/sum);
    result[2] = (float)sqrt(z*z/sum);
}

GLuint createShaderProgram(const char * vshader,const char * fshader)
{
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vshader, NULL);
  glCompileShader(vs);
  int params = GL_FALSE;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE)
    {
      int logLength;
      fprintf(stderr, "ERROR: vertex shader did not compile\n");
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0 )
        {
		  char * buffer = malloc(logLength+1);
		  glGetShaderInfoLog(vs, logLength, NULL, buffer);
		  printf("%s\n", buffer);
	    }
      return 0;
    }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fshader, NULL);
  glCompileShader(fs);
  params = GL_FALSE;
  glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE)
    {
      int logLength;
      fprintf(stderr, "ERROR: fragment shader did not compile\n");
      glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0 )
        {
		  char * buffer = malloc(logLength+1);
		  glGetShaderInfoLog(fs, logLength, NULL, buffer);
		  printf("%s\n", buffer);
	    }
      return 0;
    }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fs);
  glAttachShader(shaderProgram, vs);
  glLinkProgram(shaderProgram);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    {
      fprintf(stderr,"ERROR: Could not create the shaders\n");
      return 0;
    }
  return shaderProgram;
}

GLFWwindow * init(int width,int height,char * title)
{
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow * window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (window == NULL)
    return NULL;

  glfwMakeContextCurrent(window);

  glEnable(GL_DEPTH_TEST); // enable z buffer
  glDepthFunc(GL_LESS);

    // don't display the back (improves performance)
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  return window;
}

int loadImage(char * fileName)
{
  int width, height, numChans;
  GLubyte *data = stbi_load(fileName, &width, &height, &numChans, 0);
  if (data != NULL)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  else
   {
     fprintf(stderr,"Failed to load texture %s %s\n", fileName, stbi_failure_reason());
     return 0;
   }
  //stbi_image_free(data);
  return 1;
}

const char * loadFile(const char * fileName)
{
 struct stat fileInfo;
 unsigned fileSize;
 FILE * file;
 char * buf;

 file = fopen(fileName,"r");
 if (file == NULL)
   return NULL;
 stat(fileName,&fileInfo);
 fileSize = fileInfo.st_size;
 buf = malloc(fileSize+1);
 fread(buf,1,fileSize,file);
 fclose(file);
 buf[fileSize] = '\0';
 return buf;
}

//end of boilerplate

const float roofPoints[] = { // vertex xyz, solid color rgb, u&v

//start1st

//A
0,3,-1.5f,
0,0,0,
0,0,
//B
-1.5f,2,-1.5f,
0,0,0,
0,0,
//C
1.5f,2,-1.5f,
0,0,0,
0,0,
//E
-1.5f,2,1.5f,
0,0,0,
0,0,
//F
1.5f,2,1.5f,
0,0,0,
0,0,
//D
0,3,1.5f,
0,0,0,
0,0,
//start2nd

//C
1.5f,2,-1.5f,
0,0,0,
0,0,
//F
1.5f,2,1.5f,
0,0,0,
0,0,
//A
0,3,-1.5f,
0,0,0,
0,0,
//D
0,3,1.5f,
0,0,0,
0,0,
//B
-1.5f,2,-1.5f,
0,0,0,
0,0,

//E
-1.5f,2,1.5f,
0,0,0,
0,0
};

const float housePoints[] = {   // vertex xyz, color rgb, texture u,v

  //start1st

  //A
  -1,2,-1, //A
  1,1,1,
  0,1,
  //C
  -1,0,-1, //C
  1,1,1,
  0,0,
  //B
  1,2,-1, //B
  1,1,1,
  1,1,
  //D
  1,0,-1, //D
  1,1,1,
  1,0,
  //F
  1,2,1, //F
  1,1,1,
  2,1,
  //H
  1,0,1, //H
  1,1,1,
  2,0,
  //E
  -1,2,1, //E
  1,1,1,
  3,1,
  //G
  -1,0,1, //G
  1,1,1,
  3,0,
  //A
  -1,2,-1, //A
  1,1,1,
  4,1,
  //C
  -1,0,-1, //C
  1,1,1,
  4,0,

  //start2nd

  //E
  -1,2,1, //E
  1,1,1,
  1,3,
  //A
  -1,2,-1, //A
  1,1,1,
  0,3,

  //F
  1,2,1, //F
  1,1,1,
  1,2,
  //B
  1,2,-1, //B
  1,1,1,
  0,2,




  //H
  1,0,1, //H
  1,1,1,
  1,1,
  //D
  1,0,-1, //D
  1,1,1,
  0,1,
  //G
  -1,0,1, //G
  1,1,1,
  1,0,
  //C
  -1,0,-1, //C
  1,1,1,
  0,0

  // //H
  // 1,0,1, //H
  // 1,0,0,
  // 1,1,
  // //D
  // 1,0,-1, //D
  // 0,1,0,
  // 0,1,
  // //G
  // -1,0,1, //G
  // 1,1,1,
  // 1,0,
  // //C
  // -1,0,-1, //C
  // 0,0,0,
  // 0,0
};

mat4 viewTransform;
mat4 projection;
GLuint textureShaders;
GLuint simpleShaders;
GLuint sailShaders;
GLuint sailVertices;
GLuint sailTexture;
mat4 sailTransform;
GLuint waterShaders;
GLuint waterVertices;
GLuint waterTexture;
mat4 waterTransform;
GLuint houseShaders;
GLuint houseVertices;
mat4 houseTransform;
GLuint roofShaders;
GLuint roofVertices;
mat4 roofTransform;
int simpleProjectLoc;
int simpleViewLoc;
int simpleModelLoc;
int waterProjectLoc;
int waterViewLoc;
int waterModelLoc;
int waterTextureLoc;
int textureTextureLoc;
int textureViewLoc;
int textureModelLoc;
int textureTextureLoc;
int sailProjectLoc;
int sailViewLoc;
int sailModelLoc;
int roofProjectLoc;
int roofViewLoc;
int roofModelLoc;
int houseProjectLoc;
int houseViewLoc;
int houseModelLoc;
int sailTextureLoc;
int sailAngleLoc;
int sailWindDirLoc;
int sailWindSpeedLoc;
int waterTextureLoc;
int waterAngleLoc;
int waterWindDirLoc;
int waterWindSpeedLoc;
int houseTextureLoc;
int houseAngleLoc;
int houseWindDirLoc;
int houseWindSpeedLoc;
int roofTextureLoc;
int roofAngleLoc;
int roofWindDirLoc;
int roofWindSpeedLoc;
float currentAngle;
float windDir;
float windSpeed;
//house and roof wood texture declaration
GLuint roofTexture;
int roofTextureLoc;
GLuint houseTexture;
int houseTextureLoc;

mat4 temp;

void display()
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//roof

  glUseProgram(roofShaders);
  glBindVertexArray(roofVertices);


  glUniformMatrix4fv(roofProjectLoc, 1, GL_TRUE, (float *)projection);
  glUniformMatrix4fv(roofViewLoc, 1, GL_TRUE, (float *)viewTransform);
  glUniformMatrix4fv(roofModelLoc, 1, GL_TRUE, (float *)temp);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, roofTexture);
  glUniform1i(roofTextureLoc,0);

  //glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);

//house

  glUseProgram(houseShaders);
  glBindVertexArray(houseVertices);


  glUniformMatrix4fv(houseProjectLoc, 1, GL_TRUE, (float *)projection);
  glUniformMatrix4fv(houseViewLoc, 1, GL_TRUE, (float *)viewTransform);
  glUniformMatrix4fv(houseModelLoc, 1, GL_TRUE, (float *)temp);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, houseTexture);
  glUniform1i(houseTextureLoc,0);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
  glDrawArrays(GL_TRIANGLE_STRIP, 10, 8);

}


const char * basicVertexShader =
"#version 400\n"
"uniform mat4 project;"
"uniform mat4 view;"
"uniform mat4 model;"
"layout(location=0) in vec3 vertex;"
"layout(location=1) in vec3 vColor;"
"layout(location=2) in vec2 vTexCoord;"
"out vec3 color;"
"out vec2 texCoord;"
"void main() {"
"  gl_Position = project*view*model*vec4(vertex, 1.0);"
"  color = vColor;"
"  texCoord = vTexCoord;"
"}";

const char * textureFragmentShader =
"#version 400\n"
"uniform sampler2D textureMap;"
"in vec3 color;"
"in vec2 texCoord;"
"out vec4 frag_color;"
"void main() {"
"  frag_color = vec4(color,1.)*texture(textureMap, texCoord);"
"}";

const char * colorFragmentShader =
"#version 400\n"
"uniform sampler2D textureMap;"
"in vec3 color;"
"in vec2 texCoord;"
"out vec4 frag_color;"
"void main() {"
"  frag_color = vec4(color,1.);"
"}";

int main(int argc,char ** argv)
{
  int width = 800;
  int height = 800;
  float baseDir = d2r(0);
  const char * sailVShader = NULL;
  const char * waterVShader = NULL;
  for (int i = 1; i < argc; i++)
  {
      if (argv[i][0] == '-')
      {
        if (strcmp(argv[i],"-w") == 0)
        {
            width = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i],"-h") == 0)
        {
            height = atoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i],"-dir") == 0)
        {
            baseDir = d2r(atoi(argv[i+1]));
            i++;
        }
        else if (strcmp(argv[i],"-sail") == 0)
        {
            sailVShader = loadFile(argv[i+1]);
            i++;
            if (sailVShader == NULL)
              {
                printf("could not load sail vertex shader %s\n",argv[i]);
                return 11;
              }
            printf("Using sail vertex shader\n%s\n",sailVShader);
        }
        else if (strcmp(argv[i],"-water") == 0)
        {
            waterVShader = loadFile(argv[i+1]);
            i++;
            if (waterVShader == NULL)
              {
                printf("could not load water vertex shader %s\n",argv[i]);
                return 11;
              }
            printf("Using water vertex shader\n%s\n",waterVShader);
        }
        else
        {
            printf("Unknown switch %s\n",argv[i]);
            return 7;
        }
        continue;
      }
      printf("Unexpected argument %s\n",argv[i]);
      return 17;
  }

  GLFWwindow* window = init(width,height,"Brick House With Roof");
  if (window == NULL) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }

  textureShaders = createShaderProgram(basicVertexShader,textureFragmentShader);
  if (textureShaders == 0)
    return 2;
  textureTextureLoc = glGetUniformLocation(textureShaders,"textureMap");

  if (waterVShader == NULL)
    {
     waterShaders = textureShaders;
     waterTextureLoc = textureTextureLoc;
    }

  else
  {
      waterShaders = createShaderProgram(waterVShader,textureFragmentShader);
      if (waterShaders == 0)
        return 2;
      waterTextureLoc = glGetUniformLocation(waterShaders,"textureMap");
    }
  waterModelLoc = glGetUniformLocation(waterShaders,"model");
  waterViewLoc = glGetUniformLocation(waterShaders,"view");
  waterProjectLoc = glGetUniformLocation(waterShaders,"project");
  waterAngleLoc = glGetUniformLocation(waterShaders,"currentAngle");
  waterWindDirLoc = glGetUniformLocation(waterShaders,"windDir");
  waterWindSpeedLoc = glGetUniformLocation(waterShaders,"windSpeed");

//here we make sure the house, roof and sail all follow the shader as well

    if (sailVShader == NULL)
    {
      sailShaders = textureShaders;
      sailTextureLoc = textureTextureLoc;
    }
  else
    {
      sailShaders = createShaderProgram(sailVShader,textureFragmentShader);
      if (sailShaders == 0)
        return 2;
      sailTextureLoc = glGetUniformLocation(sailShaders,"textureMap");
    }
  sailAngleLoc = glGetUniformLocation(sailShaders,"currentAngle");
  sailWindDirLoc = glGetUniformLocation(sailShaders,"windDir");
  sailWindSpeedLoc = glGetUniformLocation(sailShaders,"windSpeed");
  sailModelLoc = glGetUniformLocation(sailShaders,"model");
  sailViewLoc = glGetUniformLocation(sailShaders,"view");
  sailProjectLoc = glGetUniformLocation(sailShaders,"project");

  simpleShaders = createShaderProgram(basicVertexShader,colorFragmentShader);
  if (simpleShaders == 0)
    return 2;
  simpleModelLoc = glGetUniformLocation(simpleShaders,"model");
  simpleViewLoc = glGetUniformLocation(simpleShaders,"view");
  simpleProjectLoc = glGetUniformLocation(simpleShaders,"project");

  roofShaders = simpleShaders;
  houseShaders = textureShaders;

  roofModelLoc = glGetUniformLocation(roofShaders,"model");
  roofViewLoc = glGetUniformLocation(roofShaders,"view");
  roofProjectLoc = glGetUniformLocation(roofShaders,"project");

  houseModelLoc = glGetUniformLocation(houseShaders,"model");
  houseViewLoc = glGetUniformLocation(houseShaders,"view");
  houseProjectLoc = glGetUniformLocation(houseShaders,"project");
  houseTextureLoc = glGetUniformLocation(houseShaders,"textureMap");


//roof

  glGenVertexArrays(1, &roofVertices);
  glBindVertexArray(roofVertices);


  // put the vertices in a vertex buffer object
  GLuint vertices_vbo = 0;
  glGenBuffers(1, &vertices_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
  glBufferData(GL_ARRAY_BUFFER,
                sizeof(roofPoints),
                roofPoints,
                GL_STATIC_DRAW);
  // and describe it as the first argument to the vertex shader
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(roofPoints[0]), 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(roofPoints[0]), (void*)(3 * sizeof(roofPoints[0])));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                        8 * sizeof(roofPoints[0]),
                        (void*)(6 * sizeof(roofPoints[0])));
  glEnableVertexAttribArray(2);

  // glGenVertexArrays(1, &roofVertices);
  // glBindVertexArray(roofVertices);

//house

//  create a vertex array object to use the buffers
glGenVertexArrays(1, &houseVertices);
glBindVertexArray(houseVertices);


// put the vertices in a vertex buffer object
vertices_vbo = 0;
glGenBuffers(1, &vertices_vbo);
glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
glBufferData(GL_ARRAY_BUFFER,
             sizeof(housePoints),
             housePoints,
             GL_STATIC_DRAW);
// and describe it as the first argument to the vertex shader
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(housePoints[0]), 0);
glEnableVertexAttribArray(0);

glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(housePoints[0]), (void*)(3 * sizeof(housePoints[0])));
glEnableVertexAttribArray(1);

glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(housePoints[0]), (void*)(6 * sizeof(housePoints[0])));
glEnableVertexAttribArray(2);

//textures

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenTextures(1, &houseTexture);
  glBindTexture(GL_TEXTURE_2D, houseTexture);
  if (!loadImage("brick.jpg"))  // loads it into the current texture
    return 3;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  identity(temp);

  mat4 roofScale;
  mat4 roofTranslate;
  scale(0.2f,1,0.2f,roofScale);
  translate(-0.5f,0,0,roofTranslate);
  mat4Mult(roofTranslate,roofScale,roofTransform);
  mat4 houseScale;
  mat4 houseTranslate;
  scale(0.2f,1,0.2f,houseScale);
  translate(-0.5f,0,0,houseTranslate);
  mat4Mult(houseTranslate,houseScale,houseTransform);
  mat4 worldRotate;
  rotateX(d2r(90),worldRotate);
  rotateY(d2r(-45),worldRotate);
  mat4 worldTranslate;
  translate(0,-2,-7,worldTranslate);
  mat4Mult(worldTranslate,worldRotate,viewTransform);
  perspectivea(24,(float)width/height,1,100,projection);
  glClearColor(0.678f,0.847f,0.902f,1);
  while(!glfwWindowShouldClose(window))
  {
    float time = glfwGetTime();
    currentAngle = (int)(time*1000)%6283/1000.0f;

    rotateY(d2r(time*25),temp);


    windDir = (float)sin(time*3.81+0.2)*.1f+baseDir;
    windSpeed = (float)cos(time/27.2)*20+10;
    display();
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
