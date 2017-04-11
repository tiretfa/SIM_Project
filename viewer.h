#ifndef VIEWER_H
#define VIEWER_H

// GLEW lib: needs to be included first!!
#include <GL/glew.h> 

// OpenGL library 
#include <GL/gl.h>

// OpenGL Utility library
#include <GL/glu.h>

// OpenGL Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QGLFormat>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <stack>

#include "camera.h"
#include "meshLoader.h"
#include "grid.h"
#include "shader.h"

class Viewer : public QGLWidget {
 public:
  Viewer(const QGLFormat &format=QGLFormat::defaultFormat());
  ~Viewer();

 protected :
  virtual void paintGL();
  virtual void initializeGL();
  virtual void resizeGL(int width,int height);
  virtual void keyPressEvent(QKeyEvent *ke);
  virtual void mousePressEvent(QMouseEvent *me);
  virtual void mouseMoveEvent(QMouseEvent *me);
    

 private:
  void createVAO();
  void deleteVAO();
  void loadGridIntoVAO();
  void drawVAO();

  void initFBO();
  void createFBO();
  void deleteFBO();

  void createShader();
  void deleteShader();
  void enableShader();
  void disableShader();


  bool           _drawMode; // press w for wire or fill drawing mode
  //Mesh  *_grid;
  Grid  *_grid;
  Camera *_cam;    // the camera
  Shader *_shaderPerlinNoisePass; // the shader
  Shader *_shaderNormalPass;

  std::string _vertexFilename;
  std::string _fragmentFilename;

  GLuint _vao;
  GLuint _buffers[3];

  //render texture ids
  GLuint _rendPerlinId;
  GLuint _rendNormalId;
  GLuint _rendDepthId;

  // fbo id
  GLuint _fbo;

};

#endif // VIEWER_H
