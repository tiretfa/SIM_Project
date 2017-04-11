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
#include "shader.h"

class Viewer : public QGLWidget {
 public:
  Viewer(char *filename,
	 const QGLFormat &format=QGLFormat::defaultFormat());
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
  void loadMeshIntoVAO();
  void drawVAO();

  void createShader();
  void deleteShader();
  void enableShader();
  void disableShader();

  QTimer        *_timer;    // timer that controls the animation
  bool           _drawMode; // press w for wire or fill drawing mode
  float          _var;      // animated variable (going into [0-1-0-1...1-0])
  float          _speed;    // speed added to the spin at each frame

  Mesh   *_mesh;   // the mesh
  Camera *_cam;    // the camera
  Shader *_shader; // the shader

  std::string _vertexFilename;
  std::string _fragmentFilename;

  GLuint _vao;
  GLuint _buffers[3];
};

#endif // VIEWER_H
