#define MAX_LIGHTS 4													
attribute vec3 a_position;
attribute vec2 a_texCoord;
attribute vec3 a_normal;

uniform mat4 CC_NormalMatrix;
uniform bool uTexture;
uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform bool uLightEnabled[MAX_LIGHTS];
uniform vec3 uLightAmbience[MAX_LIGHTS];
uniform vec3 uLightDiffuse[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];

uniform float alpha;
uniform int mode;

varying vec4 v_color;

void main()
{																			
	vec3 defaultAmbience = vec3(0.05);		
  								
	// all following gemetric computations are performed in the				
	// camera coordinate system (aka eye coordinates)			
	
	highp vec3 normal = vec3(CC_NormalMatrix * vec4(a_normal, 0.0));
	vec4 vertPos4 = CC_MVMatrix * vec4(a_position, 1.0);

	highp vec3 frontColor = vec3(0.0);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{	
		highp vec3 vertPos = vec3(vertPos4) / vertPos4.w;
		highp vec3 lightDir = normalize(uLightPosition[i] - vertPos);
		highp vec3 reflectDir = reflect(-lightDir, normal);
		highp vec3 viewDir = normalize(vertPos);
		float lambertian = max(dot(lightDir,normal), 0.0);
		float specular = 0.0;
		
		if(lambertian > 0.0)
		{
			float specAngle = max(dot(reflectDir, viewDir), 0.0);
			specular = pow(specAngle, 4.0);
			
			// the exponent controls the shininess (try mode 2)
			if(mode == 2)  specular = pow(specAngle, 1.0);
			
			// according to the rendering equation we would need to multiply
			// with the the lambertian, but this has little visual effect
			
			if(mode == 3) specular *= lambertian;
			
			// switch to mode 4 to turn off the specular component
			
			if(mode == 4) specular *= 0.0;											
		}

		if (uLightEnabled[i])
			frontColor += vec3(defaultAmbience*uLightAmbience[i] + lambertian*uDiffuse*uLightDiffuse[i] + specular*uSpecular) * uLightIntensity[i];
	}
	
	v_color = vec4(frontColor,alpha);

	gl_Position = CC_MVPMatrix * vec4(a_position, 1.0);
}
