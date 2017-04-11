#version 330

out vec4 bufferColor;


uniform vec3 myColor;

uniform vec3 myOtherColor;



void main() {

  // re-normalize 
  //vec3 color = mix(vec3(sin(var)*0.5+0.5,sin(var)*0.5+0.5,sin(var)*0.5+0.5),myColor,var);
  // normal coordinates are used as colors here 
  bufferColor = vec4(myColor*0.5+0.5,1.0);

  // color modified by a global variable 
  //bufferColor = vec4(color*normal,1.0);
}
