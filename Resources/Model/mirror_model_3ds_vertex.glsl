
attribute vec4 in_position;
attribute vec3 in_normal;
attribute vec2 in_texcoord;

varying vec4 diffuse,ambientGlobal,ambient;
varying vec3 normal,lightDir,halfVector;
varying float dist;
varying vec2 texcoord;

uniform vec4 lightSourcePosition;
uniform vec4 lightSourceHalfVector;
uniform vec4 lightSourceDiffuse;
uniform vec4 lightSourceAmbient;
uniform vec4 frontMaterialDiffuse;
uniform vec4 frontMaterialAmbient;
uniform vec4 lightModelAmbient;

uniform mat4 modelmatrix;
uniform mat3 cl_NormalMatrix;
uniform mat4 cl_ModelViewMatrix;
uniform mat4 cl_ModelViewProjectionMatrix;

void main()
{	
	vec4 ecPos;
	vec3 aux;
	
	normal = normalize(cl_NormalMatrix * mat3(modelmatrix) * in_normal);
	
	/* these are the new lines of code to compute the light's direction */
	ecPos = cl_ModelViewMatrix * modelmatrix * in_position;
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
	
	texcoord = in_texcoord;
	vec4 p = modelmatrix * in_position;
	gl_Position = cl_ModelViewProjectionMatrix * vec4(p.x, -p.y, p.z, p.w);
} 
