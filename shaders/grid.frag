#version 330

out vec4 bufferColor;

uniform sampler2D normalmap;

in vec2 texcoord;

void main() {

  // re-normalize 
  //vec3 color = mix(vec3(sin(var)*0.5+0.5,sin(var)*0.5+0.5,sin(var)*0.5+0.5),myColor,var);
  // normal coordinates are used as colors here 

  vec4 color = texture2D(normalmap,texcoord);

  vec3 newColor = vec3(min(1-color.w+0.2,0.5),min(max(1-color.w+0.2,0.4),0.5),1-color.w+0.2);

  bufferColor = vec4(newColor.xyz,1);

  // color modified by a global variable 
  //bufferColor = vec4(color*normal,1.0);
}
