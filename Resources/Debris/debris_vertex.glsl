
uniform mat4 cl_ModelViewMatrix;
uniform mat4 cl_ProjectionMatrix;
uniform vec4 offset;
attribute vec4 in_position;
varying float dist;

void main()
{	
	vec4 p = cl_ModelViewMatrix * (in_position+offset);
	gl_Position = cl_ProjectionMatrix * p;
	dist = gl_Position.z;
	gl_PointSize = clamp(30.0/dist, 0.5, 4.0);
} 
