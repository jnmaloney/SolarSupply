attribute vec4 vPosition;
attribute vec2 vUV;
uniform mat4 ModelLocal;
uniform mat4 ViewProj;
varying vec2 uv;
//uniform vec3 diffuse; // Palette
varying vec3 colour;   // Palette
void main()
{
   gl_Position = ViewProj * (ModelLocal * vPosition);
   uv.x = vUV.x;
   uv.y = vUV.y;
   colour = vec3(1, 1, 1);
}
