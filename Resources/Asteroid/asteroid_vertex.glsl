
uniform mat4 cl_ModelViewProjectionMatrix;
attribute vec4 in_position;
varying vec3 v;
uniform samplerCube texture1;
uniform samplerCube texture2;

void main()
{
	v = vec3(in_position);
	float scale = 0.8+textureCube(texture2, in_position.xyz).r/15+textureCube(texture2, in_position.xyz).g/15+textureCube(texture2, in_position.xyz).b/15;
	gl_Position = cl_ModelViewProjectionMatrix * (in_position*vec4(scale,scale,scale,1.0f));
}
