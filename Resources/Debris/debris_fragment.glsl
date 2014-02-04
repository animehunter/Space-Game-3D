
varying float dist;

void main()
{
	gl_FragColor = vec4(0.5, 0.5, 0.5, (1.0-dist/100));
}
