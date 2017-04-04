#include "viewer.h"

#include <math.h>
#include <iostream>
#include "grid.h"

#include <QTime>

using namespace std;


Viewer::Viewer(const QGLFormat &format)
  : QGLWidget(format),
    _timer(new QTimer(this)),
    _currentshader(0),
    _light(glm::vec3(0,0,1)),
    _mode(false) {

  setlocale(LC_ALL,"C");

  _grid = new Grid(1024,-1.0,1.0);
  _cam  = new Camera(1024,glm::vec3(0,0,0));

  _timer->setInterval(10);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  delete _timer;
  delete _grid;
  delete _cam;

  // delete all GPU objects  
  deleteShaders();
  deleteVAO(); 
  deleteFBO();
}

void Viewer::deleteVAO() {
  // delete VAOs
  glDeleteBuffers(1,_buffers);
  glDeleteBuffers(1,&_quad);

  glDeleteVertexArrays(1,&_vaoObject);
  glDeleteVertexArrays(1,&_vaoQuad);
}
/*
void Viewer::createFBO() {
  // Ids needed for the FBO and associated textures 
  glGenFramebuffers(1,&_fbo);
  glGenTextures(1,&_rendNormalId);
  glGenTextures(1,&_rendColorId);
  glGenTextures(1,&_rendDepthId);
}

void Viewer::initFBO() {

 // create the texture for rendering colors
  glBindTexture(GL_TEXTURE_2D,_rendColorId);
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

  // attach textures to framebuffer object 
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);
  glBindTexture(GL_TEXTURE_2D,_rendColorId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,_rendColorId,0);
  glBindTexture(GL_TEXTURE_2D,_rendNormalId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,_rendNormalId,0);
  glBindTexture(GL_TEXTURE_2D,_rendDepthId);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,_rendDepthId,0);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void Viewer::deleteFBO() {
  // delete all FBO Ids
  glDeleteFramebuffers(1,&_fbo);
  glDeleteTextures(1,&_rendNormalId);
  glDeleteTextures(1,&_rendColorId);
  glDeleteTextures(1,&_rendDepthId);
}
*/
void Viewer::createVAO() {
  // create VAO
  glGenVertexArrays(1,&_vaoObject);

  // create 5 associated VBOs (for positions, normals, tangents, coords and face indices)
  glGenBuffers(1,_buffers);
}
void Viewer::loadGridIntoVAO(){
  // bind VAO 
  glBindVertexArray(_vaoObject);
  
  // send and enable positions 
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nb_vertices*3*sizeof(float),_grid->vertices,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
/*

  // send and enable normals 
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[1]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nb_vertices*3*sizeof(float),_grid->normals,GL_STATIC_DRAW);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(1);

  // send and enable tangents
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[2]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nb_vertices*3*sizeof(float),_grid->tangents,GL_STATIC_DRAW);
  glVertexAttribPointer(2,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(2);

  // send and enable coords 
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[3]);
  glBufferData(GL_ARRAY_BUFFER,_grid->nb_vertices*2*sizeof(float),_grid->coords,GL_STATIC_DRAW);
  glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(3);

  // send faces 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_buffers[4]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_grid->nb_faces*3*sizeof(unsigned int),_grid->faces,GL_STATIC_DRAW);


  // create the VAO associated with a simple quad
  // 2 triangles that cover the viewport (a bit like in TP1)
  static const GLfloat quadData[] = { 
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
  };
  
  // create VAO
  glGenVertexArrays(1,&_vaoQuad);&_vao
  glGenBuffers(1,&_quad);

  // bind VAO 
  glBindVertexArray(_vaoQuad);

  // send and enable vertices
  glBindBuffer(GL_ARRAY_BUFFER,_quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData),quadData,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);
*/
  // back to normal
  glBindVertexArray(0);
}

void Viewer::createShaders() {

    _shader = new Shader();
    _shader->load("shaders/grid_shader.vert","shaders/grid_shader.frag");
/*
  // create 2 shaders: one for the first pass, one for the second pass 
  _shaderFirstPass = new Shader();
  _shaderSecondPass = new Shader();

  _shaderFirstPass->load("shaders/first-pass.vert","shaders/first-pass.frag");
  _shaderSecondPass->load("shaders/second-pass.vert","shaders/second-pass.frag");
*/
}

void Viewer::deleteShaders() {
    delete _shader;
/*
    delete _shaderFirstPass;  _shaderFirstPass = NULL;
  delete _shaderSecondPass; _shaderSecondPass = NULL;
  */
}


void Viewer::drawObject(const glm::vec3 &pos,const glm::vec3 &col) {
  // shader id
  const int id = _shaderFirstPass->id();

  // send uniform (constant) variables to the shader 
  glm::mat4 mdv = glm::translate(_cam->mdvMatrix(),pos);
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(mdv[0][0]));
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));
  glUniformMatrix3fv(glGetUniformLocation(id,"normalMat"),1,GL_FALSE,&(_cam->normalMatrix()[0][0]));
  glUniform3fv(glGetUniformLocation(id,"color"),1,&(col[0]));

  // activate faces and draw!
  glBindVertexArray(_vaoObject);
  glDrawElements(GL_TRIANGLES,3*_grid->nb_faces,GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::drawQuad() {
  // shader id
  const int id = _shaderSecondPass->id();

  // send shader parameters 
  glUniform3fv(glGetUniformLocation(id,"light"),1,&(_light[0]));

  // send textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,_rendColorId);
  glUniform1i(glGetUniformLocation(id,"colormap"),0);

  glActiveTexture(GL_TEXTURE0+1);
  glBindTexture(GL_TEXTURE_2D,_rendNormalId);
  glUniform1i(glGetUniformLocation(id,"normalmap"),1);

  // Draw the 2 triangles !
  glBindVertexArray(_vaoQuad);
  glDrawArrays(GL_TRIANGLES,0,6);
  glBindVertexArray(0);
}

void Viewer::paintGL() {
  // activate the created framebuffer object
  glBindFramebuffer(GL_FRAMEBUFFER,_fbo);

  // activate the shader 
  glUseProgram(_shaderFirstPass->id());

  GLenum bufferlist [] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};

  glDrawBuffers(2,bufferlist);

  // clear buffers 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw multiple objects 
  const float r = _grid->radius*2.5;
  const int   v = 5;
  for(int i=-v;i<=v;++i) {
    for(int j=-v;j<=v;++j) {
      drawObject(glm::vec3(i*r,0,j*r),glm::vec3((float)(i+v)/(float)(2*v+1),0.5,(float)(j+v)/(float)(2*v+1)));
    }
  }

  // desactivate fbo
  glBindFramebuffer(GL_FRAMEBUFFER,0);

  // clear everything
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // activate the shader 
  glUseProgram(_shaderSecondPass->id());

  // Draw the triangles !
  drawQuad();

  // disable shader 
  glUseProgram(0);

}

void Viewer::resizeGL(int width,int height) {
  _cam->initialize(width,height,false);
  glViewport(0,0,width,height);

  // re-init the FBO (textures need to be resized to the new viewport size)
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
  
  // key a: play/stop animation
  if(ke->key()==Qt::Key_A) {
    if(_timer->isActive()) 
      _timer->stop();
    else 
      _timer->start();
  }

  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(),height(),true);
  }
  
  // key r: reload shaders 
  _shaderFirstPass->reload("shaders/first-pass.vert","shaders/first-pass.frag");
  _shaderSecondPass->reload("shaders/second-pass.vert","shaders/second-pass.frag");

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  // init and check glew
  glewExperimental = GL_TRUE;

  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glViewport(0,0,width(),height());

  // initialize camera
  _cam->initialize(width(),height(),true);

  // load shader files
  createShaders();

  // init VAO
  createVAO();
  loadGridIntoVAO();

  // create/init FBO
  createFBO();
  initFBO();

  // starts the timer 
  _timer->start();
}

