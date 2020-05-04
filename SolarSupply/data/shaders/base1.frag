precision mediump float;
//uniform sampler2D BaseMap;
//varying vec2 uv;
varying vec3 colour;
void main()
{
//  vec3 shade = texture2D(BaseMap, uv).rgb;
//  gl_FragColor = vec4(colour * shade, 1.0);
  gl_FragColor = vec4(colour, 1.0);
}
