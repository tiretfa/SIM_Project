#version 330

// starting from OpenGL 3.3 it is possible to use this syntax to make the relations with arrays
layout(location = 0) in vec3 position;


uniform mat4 mvp; // modelview projection matrix (constant for all the vertices)




void main() {
  vec3 pos = vec3(position.x,position.y,position.z);
    //vec3 pos = vec3(position.x*2*sin(var)+2*smoothstep(position.z,sin(1+var),cos(2-var)),position.y,position.z);
  //vec3 pos = vec3(mix(position.xy,(position.x+position.y)/2.0),position.y,position.z);
    gl_Position = mvp*vec4(pos,1.0);

}
