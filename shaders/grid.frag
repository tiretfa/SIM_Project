#version 330

out vec4 bufferColor;

uniform sampler2D normalmap;
uniform vec3 light;
uniform mat3 normalMat;

in vec2 texcoord;
in vec3 rasterizeNormal;

void main() {

  // re-normalize 
  //vec3 color = mix(vec3(sin(var)*0.5+0.5,sin(var)*0.5+0.5,sin(var)*0.5+0.5),myColor,var);
  // normal coordinates are used as colors here 

  vec4 color = texture2D(normalmap,texcoord);
  vec3 nCam = normalMat * rasterizeNormal;
  vec3 nCamNormalize = normalize(nCam);
  float normalLight = max(dot(nCamNormalize,light),0);
  vec3 ambientCam = vec3(0.25,0.25,0.25);
  vec3 diffuseCam = vec3(1,1,1)*normalLight;
  float reflected = max(dot(reflect(light,nCamNormalize),vec3(0,0,-1)),0);
  vec3 specularCam = vec3(1,1,1)* pow(reflected,1.0);
  vec3 phong = ambientCam + diffuseCam + specularCam;

  bufferColor = vec4(phong,1);

  // color modified by a global variable 
  //bufferColor = vec4(color*normal,1.0);
}
