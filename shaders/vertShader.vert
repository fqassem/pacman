varying float normalDotLight;
varying vec4 diffuse;
varying vec4 col;

void main()
{
	/* Blinn-Phong uses light's half-vector */
	vec3 halfVector = gl_LightSource[0].halfVector.xyz;
	vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
	vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
	normalDotLight = max(dot(normal, lightDir), 0.0); //clamped
	
	vec4 specular = vec4(0.0);

	diffuse = gl_Color * gl_LightSource[0].diffuse;
	vec4 ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	vec4 globalAmbient = gl_LightModel.ambient * gl_FrontMaterial.ambient;
	
	float normalDotHalfVector;
	if(normalDotLight > 0.0)
	{
		normalDotHalfVector = max(dot(normal, normalize(halfVector)), 0.0);
		specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(normalDotHalfVector, gl_FrontMaterial.shininess);
	}
	
	col = globalAmbient + ambient + specular;
	gl_Position = ftransform();
} 
