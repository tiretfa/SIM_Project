#include "viewer.h"

#include <math.h>
#include <iostream>
#include "meshLoader.h"
#include <QTime>

using namespace std;

Viewer::Viewer(const QGLFormat &format)
  : QGLWidget(format),
    _drawMode(false),
    _light(glm::vec3(0,0,1)),
    _mode(false)
    {

  // load a mesh into the CPU memory
  _grid = new Grid(1024,-1.0,1.0);

  _cam  = new Camera(3,glm::vec3(0,0,0));
  _deplacement;
}

Viewer::~Viewer() {
  // delete everything 
  delete _grid;
  delete _cam;

  deleteFBO();
  deleteVAO();
  deleteShader();
}

void Viewer::createShader() {
  _shaderPerlinNoisePass = new Shader();
  _shaderNormalPass = new Shader();
  _shaderGridPass = new Shader();

  _shaderPerlinNoisePass->load("shaders/noise.vert","shaders/noise.frag");
  _shaderNormalPass->load("shaders/normal.vert","shaders/normal.frag");
  _shaderGridPass->load("shaders/grid.vert","shaders/grid.frag");

}

void Viewer::deleteShader() {
  delete _shaderPerlinNoisePass; _shaderPerlinNoisePass = NULL;
  delete _shaderNormalPass; _shaderNormalPass = NULL;
  delete _shaderGridPass; _shaderGridPass = NULL;
}

void Viewer::createVAO() {


  const GLfloat quadData[] = {
    -1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f };

  glGenBuffers(2,_terrain);
  glGenBuffers(1,&_quad);
  glGenVertexArrays(1,&_vaoTerrain);
  glGenVertexArrays(1,&_vaoQuad);

  // create the VBO associated with the grid (the terrain)
  glBindVertexArray(_vaoTerrain);
  glBindBuffer(GL_ARRAY_BUFFER,_terrain[0]); // vertices
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_terrain[1]); // indices
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(int),_grid->faces(),GL_STATIC_DRAW);

  // create the VBO associated with the screen quad
  glBindVertexArray(_vaoQuad);
  glBindBuffer(GL_ARRAY_BUFFER,_quad); // vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData),quadData,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
}

void Viewer::deleteVAO() {
  glDeleteBuffers(2,_terrain);
  glDeleteBuffers(1,&_quad);
  glDeleteVertexArrays(1,&_vaoTerrain);
  glDeleteVertexArrays(1,&_vaoQuad);
}
void Viewer::createFBO() {
  // Ids needed for the FBO and associated textures
  glGenFramebuffers(1,&_fbo);
  glGenTextures(1,&_rendPerlinId);
  glGenTextures(1,&_rendNormalId);
  glGenTextures(1,&_rendDepthId);

}

void Viewer::initFBO() {

 // create the texture for rendering perlin
  glBindTexture(GL_TEXTURE_2D,_rendPerlinId);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F,width(),height(),0,GL_RGBA,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // create the texture for rendering normals
  glBindTexture(GL_TEXTURE_2D,_rendNormalId);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F,width(),height(),0,GL_RGBA,GL_FLOAT,NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // create the texture for rendering depth values
    glBindTexture(GL_TEXTURE_2D,_rendDepthId);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,width(),height(),0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  glBindTexture(GL_TEXTURE_2D,_rendPerlinId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_rendPerlinId,0);
  glBindTexture(GL_TEXTURE_2D,_rendNormalId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,_rendNormalId,0);
  glBindTexture(GL_TEXTURE_2D,_rendDepthId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_rendDepthId,0);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
      std::cout << "Le fbo est cassé, aled" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER,0);

}

void Viewer::deleteFBO() {
  // delete all FBO Ids
  glDeleteFramebuffers(1,&_fbo);
  glDeleteTextures(1,&_rendNormalId);
  glDeleteTextures(1,&_rendPerlinId);
  glDeleteTextures(1,&_rendDepthId);
}

void Viewer::loadGridIntoVAO() {
  // activate VAO
  glBindVertexArray(_vaoTerrain);
  
  // store mesh positions into buffer 0 inside the GPU memorycreate
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);

  // store mesh indices into buffer 2 inside the GPU memory
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(unsigned int),_grid->faces(),GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Viewer::drawVAO() {
  // activate the VAO, draw the associated triangles and desactivate the VAO
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::drawQuad(){

    // Draw the 2 triangles !
    glBindVertexArray(_vaoQuad);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void Viewer::enableShader(unsigned int shader) {

    GLuint id = shader;
    // get the current modelview and projection matrices

  glm::mat4 p  = _cam->projMatrix();
  glm::mat4 mv  = _cam->mdvMatrix();

  // compute the resulting transformation matrix
  glm::mat4 mvp = p*mv;

  glUseProgram(_shaderGridPass->id());

  // send the transformation matrix
  glUniformMatrix4fv(glGetUniformLocation(id,"mvp"),1,GL_FALSE,&(mvp[0][0]));

  glUniform3f(glGetUniformLocation(id,"light"),_light.x, _light.y, _light.z);

  // send the normal matrix (top-left 3x3 transpose(inverse(MDV)))
  glUniformMatrix3fv(glGetUniformLocation(id,"normalMat"),1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));
}

void Viewer::disableShader() {
  // desactivate all shaders 
  glUseProgram(0);
}

void Viewer::paintGL() {
  glViewport(0,0,width(),height());

  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);

  GLenum bufferPerlin [] = {GL_COLOR_ATTACHMENT0};

  glDrawBuffers(1,bufferPerlin);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_shaderPerlinNoisePass->id());
  glUniform2f(glGetUniformLocation(_shaderPerlinNoisePass->id(),"deplacement"),_deplacement.x,_deplacement.y);
  drawQuad();

  glBindFramebuffer(GL_FRAMEBUFFER,0);

  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  GLenum bufferNormal [] = {GL_COLOR_ATTACHMENT1};

  glDrawBuffers(1,bufferNormal);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(_shaderNormalPass->id());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_rendPerlinId);
  glUniform1i(glGetUniformLocation(_shaderNormalPass->id(),"heightmap"),0);

  drawQuad();
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(_shaderGridPass->id());
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_rendNormalId);
  glUniform1i(glGetUniformLocation(_shaderGridPass->id(),"normalmap"),0);

  enableShader(_shaderGridPass->id());

  drawVAO();
  glUseProgram(0);



}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  glViewport(0,0,width,height);
  initFBO();
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
    const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

     if(me->button()==Qt::LeftButton) {
       _cam->initRotation(p);
       _mode = false;
     } else if(me->button()==Qt::MidButton) {
       _cam->initMoveZ(p);
       _mode = false;
     } else if(me->button()==Qt::RightButton) {
       _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
       _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
       _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
       _light = glm::normalize(_light);
       _mode = true;
     }

     updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
    const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

      if(_mode) {
        // light mode
        _light[0] = (p[0]-(float)(width()/2))/((float)(width()/2));
        _light[1] = (p[1]-(float)(height()/2))/((float)(height()/2));
        _light[2] = 1.0f-std::max(fabs(_light[0]),fabs(_light[1]));
        _light = glm::normalize(_light);
      } else {
        // camera mode
        _cam->move(p);
      }

      updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  
  // key w: wire/filled
  if(ke->key()==Qt::Key_W) {
    if(!_drawMode) 
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    else 
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    
    _drawMode = !_drawMode;
  }  else if(ke->key()==Qt::Key_Q){
      _deplacement.x += 0.005 ;
  }else if(ke->key()==Qt::Key_D){
      _deplacement.x -= 0.005;
  }else if(ke->key()==Qt::Key_Z){
      _deplacement.y += 0.005;
  }else if(ke->key()==Qt::Key_S){
      _deplacement.y -= 0.005;
  }


  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // key r: reload shaders 
  if(ke->key()==Qt::Key_R) {
    //_shader->reload(_vertexFilename.c_str(),_fragmentFilename.c_str());
  }

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  glewExperimental = GL_TRUE;

  // init and chack glew
  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // initialize camera
  _cam->initialize(width(),height(),true);

  createFBO();
  initFBO();

  createShader();
  createVAO();


}

