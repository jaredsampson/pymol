varying float fog;

void main()
{
  vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_ClipVertex = vertex;
  gl_FogFragCoord = length(vertex.z);
  fog = (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale;
  gl_Position = ftransform();
}
