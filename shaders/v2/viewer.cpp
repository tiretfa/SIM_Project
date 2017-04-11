#include "viewer.h"

#include <math.h>
#include <iostream>
#include "meshLoader.h"
#include <QTime>

using namespace std;

Viewer::Viewer(char *filename,const QGLFormat &format)
  : QGLWidget(format),
    _timer(new QTimer(this)),
    _drawMode(false),
    _var(0.0f),
    _speed(0.1f) {

  // load a mesh into the CPU memory
  _mesh = new Mesh(filename);
  
  // create a camera (automatically modify model/view matrices according to user interactions)
  _cam  = new Camera(_mesh->radius,glm::vec3(_mesh->center[0],_mesh->center[1],_mesh->center[2]));

  _timer->setInterval(10);
  connect(_timer,SIGNAL(timeout()),this,SLOT(updateGL()));
}

Viewer::~Viewer() {
  // delete everything 
  delete _timer;
  delete _mesh;
  delete _cam;

  deleteVAO();
  deleteShader();
}

void Viewer::createShader() {
  _shader = new Shader();
  _vertexFilename   = "shaders/helloworld.vert";
  _fragmentFilename = "shaders/helloworld.frag";
  _shader->load(_vertexFilename.c_str(),_fragmentFilename.c_str());
}

void Viewer::deleteShader() {
  delete _shader;
}

void Viewer::createVAO() {
  // create some buffers inside the GPU memory
  glGenVertexArrays(1,&_vao);
  glGenBuffers(3,_buffers);
}

void Viewer::deleteVAO() {
  // delete / free all GPU buffers 
  glDeleteBuffers(3,_buffers);
  glDeleteVertexArrays(1,&_vao);
}

void Viewer::loadMeshIntoVAO() {
  // activate VAO
  glBindVertexArray(_vao);
  
  // store mesh positions into buffer 0 inside the GPU memorycreate
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->vertices,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
  glEnableVertexAttribArray(0);

  // store mesh normals into buffer 1 inside the GPU memory
  glBindBuffer(GL_ARRAY_BUFFER,_buffers[1]);
  glBufferData(GL_ARRAY_BUFFER,_mesh->nb_vertices*3*sizeof(float),_mesh->normals,GL_STATIC_DRAW);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_TRUE,0,(void *)0);
  glEnableVertexAttribArray(1);

  // store mesh indices into buffer 2 inside the GPU memory
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_buffers[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,_mesh->nb_faces*3*sizeof(unsigned int),_mesh->faces,GL_STATIC_DRAW);

  // deactivate the VAO for now
  glBindVertexArray(0);
}

void Viewer::drawVAO() {
  // activate the VAO, draw the associated triangles and desactivate the VAO
  glBindVertexArray(_vao);
  glDrawElements(GL_TRIANGLES,3*_mesh->nb_faces,GL_UNSIGNED_INT,(void *)0);
  glBindVertexArray(0);
}

void Viewer::enableShader() {
  // get the current modelview and projection matrices 
  glm::mat4 p  = _cam->projMatrix();
  glm::mat4 mv  = _cam->mdvMatrix();

  // compute the resulting transformation matrix
  glm::mat4 mvp = p*mv;

  // activate the shader 
  glUseProgram(_shader->id());

  // send the transformation matrix
  glUniformMatrix4fv(glGetUniformLocation(_shader->id(),"mvp"),1,GL_FALSE,&(mvp[0][0]));

  // send another variable color
  glUniform3f(glGetUniformLocation(_shader->id(),"myColor"),0.0f,1.0f,0.0f);

  // send another variable color
  glUniform3f(glGetUniformLocation(_shader->id(),"myOtherColor"),1.0f,0.0f,0.0f);

  // send another variable
  glUniform1f(glGetUniformLocation(_shader->id(),"var"),_var);
}

void Viewer::disableShader() {
  // desactivate all shaders 
  glUseProgram(0);
}

void Viewer::paintGL() {
  // clear the color and depth buffers 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set viewport
  glViewport(0,0,width(),height());

  // tell the GPU to use this specified shader and send custom variables (matrices and others)
  enableShader();
  
  // actually draw the scene 
  drawVAO();

  // tell the GPU to stop using this shader 
  disableShader();

  // animate variables
  _var += _speed;
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
  if(ke->key()==Qt::Key_R) {
    _shader->reload(_vertexFilename.c_str(),_fragmentFilename.c_str());
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
  loadMeshIntoVAO();

  // starts the timer 
  _timer->start();
}

