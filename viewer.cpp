#include "viewer.h"

#include <math.h>
#include <iostream>
#include "meshLoader.h"
#include <QTime>

using namespace std;

Viewer::Viewer(const QGLFormat &format)
  : QGLWidget(format),
    _drawMode(false)
    {

  // load a mesh into the CPU memory
  _grid = new Grid(1024,-1.0,1.0);
  //_grid =new Mesh(filename);
  // create a camera (automatically modify model/view matrices according to user interactions)
  _cam  = new Camera(3,glm::vec3(0,0,0));

}

Viewer::~Viewer() {
  // delete everything 
  delete _grid;
  delete _cam;

  deleteVAO();
  deleteShader();
}

void Viewer::createShader() {
  _shaderPerlinNoisePass = new Shader();
  _shaderNormalPass = new Shader();

  _shaderPerlinNoisePass->load("shaders/noise.vert","shaders/noise.frag");
  _shaderNormalPass->load("shaders/normal.vert","shaders/normal.frag");

}

void Viewer::deleteShader() {
  delete _shaderPerlinNoisePass; _shaderPerlinNoisePass = NULL;
  delete _shaderNormalPass; _shaderNormalPass = NULL;
}

void Viewer::createVAO() {
  // create some buffers inside the GPU memory
  glGenVertexArrays(1,&_vao);
  glGenBuffers(2,_buffers);
}

void Viewer::deleteVAO() {
  // delete / free all GPU buffers 
  glDeleteBuffers(2,_buffers);
  glDeleteVertexArrays(1,&_vao);
}

void Viewer::createFBO() {
  // Ids needed for the FBO and associated textures
  glGenFramebuffers(1,&_fbo);
  glGenTextures(1,&_rendPerlinId);
  glGenTextures(1,&_rendNormalId);

}

void Viewer::initFBO() {

 // create the texture for rendering colors
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
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glBindTexture(GL_TEXTURE_2D,_rendDepthId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_rendDepthId,0);
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
  glBindVertexArray(_vao);
  
  // store mesh positions into buffer 0 inside the GPU memorycreate
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nbVertices()*3*sizeof(float),_grid->vertices(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
/*
  // store mesh normals into buffer 1 inside the GPU memory
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[1]);
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->normals,GL_STATIC_DRAW);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(1);
*/
  // store mesh indices into buffer 2 inside the GPU memory
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nbFaces()*3*sizeof(unsigned int),_grid->faces(),GL_STATIC_DRAW);
  // deactivate the VAO for now
  glBindVertexArray(0);
}

void Viewer::drawVAO() {
  // activate the VAO, draw the associated triangles and desactivate the VAO
  glBindVertexArray(_vao);
  glDrawElements(GL_TRIANGLES,3*_grid->nbFaces(),GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::enableShader() {
  // get the current modelview and projection matrices 
  glm::mat4 p  = _cam->projMatrix();
  glm::mat4 mv  = _cam->mdvMatrix();

  // compute the resulting transformation matrix
  glm::mat4 mvp = p*mv;
/*
  // activate the shader 
  glUseProgram(_shader->id());

  // send the transformation matrix
  glUniformMatrix4fv(glGetUniformLocation(_shader->id(),"mvp"),1,GL_FALSE,&(mvp[0][0]));

  // send another variable color
  glUniform3f(glGetUniformLocation(_shader->id(),"myColor"),0.0f,1.0f,0.0f);

  // send another variable color
  glUniform3f(glGetUniformLocation(_shader->id(),"myOtherColor"),1.0f,0.0f,0.0f);
*/

}

void Viewer::disableShader() {
  // desactivate all shaders 
  glUseProgram(0);
}

void Viewer::paintGL() {
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);

  glUseProgram(_shaderPerlinNoisePass->id());

  glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER,0);

  // set viewport
  glViewport(0,0,width(),height());

  // tell the GPU to use this specified shader and send custom variables (matrices and others)
  enableShader();
  
  // actually draw the scene 
  drawVAO();

  // tell the GPU to stop using this shader 
  disableShader();

}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {
  // handle camera events
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

  if(me->button()==Qt::LeftButton) {
    _cam->initRotation(p);
  } else if(me->button()==Qt::RightButton) {
    _cam->initMoveXY(p);
  } else if(me->button()==Qt::MidButton) {
    _cam->initMoveZ(p);
  }

  updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
  // handle camera motion
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));
 
  _cam->move(p);
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

  // create and initialize shaders and VAO 
  
  createShader();
  createVAO();
  loadGridIntoVAO();

}

