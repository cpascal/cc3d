#define MAX_LIGHTS 4
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_links;

uniform mat4 CC_MMatrix;
uniform mat4 CC_VMatrix;
uniform mat4 CC_NormalMatrix;
uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform bool uLightEnabled[MAX_LIGHTS];
uniform vec3 uLightAmbience[MAX_LIGHTS];
uniform vec3 uLightDiffuse[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];

uniform float alpha;
uniform int mode;

uniform mat4 uShadowProjectionMatrix;

varying vec4 v_color;
varying vec4 v_projectorCoord;
varying float v_distance;

void main()
{
	vec3 defaultAmbience = vec3(0.05);

	// all following gemetric computations are performed in the
	// camera coordinate system (aka eye coordinates)

	vec3 newNormal = a_normal;

	//newNormal.x += a_links.x*sin(CC_Time.x);
	//newNormal.y += a_links.y*sin(CC_Time.x);
	//newNormal.z += a_links.z*sin(CC_Time.x);

	highp vec3 normal = normalize(vec3(CC_NormalMatrix * vec4(newNormal, 0.0)));
	vec4 vertPos4 = CC_MVMatrix * vec4(a_position, 1.0);

	v_distance = vertPos4.z;

	highp vec3 frontColor = vec3(0.0);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		highp vec3 vertPos = vec3(vertPos4) / vertPos4.w;
		highp vec3 lightDir = normalize(-uLightPosition[i] - vertPos);
		highp vec3 reflectDir = reflect(-lightDir, normal);
		highp vec3 viewDir = normalize(vertPos);
		float lambertian = max(dot(lightDir,normal), 0.0);
		float specular = 0.0;

		if(lambertian > 0.0)
		{
			float specAngle = max(dot(reflectDir, viewDir), 0.0);
			specular = pow(specAngle, 4.0);

			// the exponent controls the shininess (try mode 2)
			if(mode == 2)  specular = pow(specAngle, 30.0);

			// according to the rendering equation we would need to multiply
			// with the the lambertian, but this has little visual effect

			if(mode == 3) specular *= lambertian;

			// switch to mode 4 to turn off the specular component

			if(mode == 4) specular *= 0.0;
		}

		if (uLightEnabled[i])
			frontColor += vec3(defaultAmbience*uLightAmbience[i] + lambertian*uDiffuse*uLightDiffuse[i] + specular*uSpecular) * uLightIntensity[i];	
	}
	
	vec3 new_pos = vec3(a_position);

	new_pos.x += a_links.x*sin(CC_Time.x);
	new_pos.y += a_links.y*sin(CC_Time.x);
	new_pos.z += a_links.z*sin(CC_Time.x);

	vec3 step = uDiffuse*vec3(new_pos.z/abs(a_links.z));

	float clampedR = clamp(step.x, frontColor.r, uDiffuse.r + 0.3);
	float clampedG = clamp(step.y, frontColor.g, uDiffuse.g + 0.3);
	float clampedB = clamp(step.z, frontColor.b, uDiffuse.b + 0.3);

	v_color = vec4(clampedR, clampedG, clampedB, alpha);

	v_projectorCoord = uShadowProjectionMatrix * (CC_MMatrix * vec4(new_pos, 1.0));

	gl_Position = CC_MVPMatrix * vec4(new_pos, 1.0);
}