#version 330

out vec4 bufferColor;

in vec3 rasterizedNormal; // received (rasterized) normal

uniform vec3 myColor;

uniform vec3 myOtherColor;

uniform float var;


void main() {

  // re-normalize 
  vec3 normal = normalize(rasterizedNormal);
  //vec3 color = mix(vec3(sin(var)*0.5+0.5,sin(var)*0.5+0.5,sin(var)*0.5+0.5),myColor,var);
  // normal coordinates are used as colors here 
  bufferColor = vec4(normal*0.5+0.5,1.0);

  // color modified by a global variable 
  //bufferColor = vec4(color*normal,1.0);
}
