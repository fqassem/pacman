varying float normalDotLight;
varying vec4 diffuse;
varying vec4 col;

void main()
{	
	gl_FragDepth = gl_FragCoord.z; // zbuffer
	gl_FragColor = col + (normalDotLight * diffuse);
} 
