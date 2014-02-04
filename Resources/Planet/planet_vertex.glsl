
uniform mat4 cl_ModelViewProjectionMatrix;
attribute vec4 in_position;
varying vec3 v;

varying vec4 diffuse,ambientGlobal,ambient;
varying vec3 normal,lightDir,halfVector;
varying float dist;

uniform vec4 lightSourcePosition;
uniform vec4 lightSourceHalfVector;
uniform vec4 lightSourceDiffuse;
uniform vec4 lightSourceAmbient;
uniform vec4 frontMaterialDiffuse;
uniform vec4 frontMaterialAmbient;
uniform vec4 lightModelAmbient;

uniform mat4 cl_ModelViewMatrix;
uniform mat3 cl_NormalMatrix;

void main()
{
	vec4 ecPos;
	vec3 aux;

	vec3 in_normal = vec3(in_position);
	
	normal = normalize(cl_NormalMatrix * in_normal);
	
	/* these are the new lines of code to compute the light's direction */
	ecPos = cl_ModelViewMatrix * in_position;
	aux = vec3(lightSourcePosition-ecPos);
	lightDir = normalize(aux);
	dist = length(aux);

	halfVector = normalize(lightSourceHalfVector.xyz);
	
	/* Compute the diffuse, ambient and globalAmbient terms */
	diffuse = frontMaterialDiffuse * lightSourceDiffuse;
	
	/* The ambient terms have been separated since one of them */
	/* suffers attenuation */
	ambient = frontMaterialAmbient * lightSourceAmbient;
	ambientGlobal = lightModelAmbient * frontMaterialAmbient;

	v = vec3(in_position);
	gl_Position = cl_ModelViewProjectionMatrix * in_position;
}
