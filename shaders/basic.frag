#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPosEye;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightPosEye;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
// Add these lines for toggling lights
uniform bool directionalLightEnabled;
uniform bool pointLightEnabled;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float shininess = 32.0f;
float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

void computeDirLight()
{
	if(directionalLightEnabled) {
		//compute eye space coordinates
		vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
		vec3 normalEye = normalize(normalMatrix * fNormal);

		//normalize light direction
		vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

		//compute view direction (in eye coordinates, the viewer is situated at the origin
		vec3 viewDir = normalize(- fPosEye.xyz);

		//compute ambient light
		ambient = ambientStrength * lightColor;

		//compute diffuse light
		diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

		//compute specular light
		vec3 reflectDir = reflect(-lightDirN, normalEye);
		float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
		specular = specularStrength * specCoeff * lightColor;
	}
  
}

void computePointLight() {

	if(pointLightEnabled) {
	
		vec3 normalEye = normalize(fNormal);	
	
		vec3 lightDirN = normalize(lightPosEye - fPosEye.xyz);
		
		
		vec3 viewDir = normalize(-fPosEye.xyz);
			
		vec3 lightColorP = vec3(0,0,1);

		float dist = length(lightPosEye - fPosEye.xyz);
		

		float att = 200.0f / (constant + linear * dist + quadratic * (dist * dist));

		//compute ambient light
		ambient += att*ambientStrength * lightColorP;
		
		//compute diffuse light
		diffuse += att*max(dot(normalEye, lightDirN), 0.0f) * lightColorP;
		
		//compute specular light

		vec3 reflectDir = reflect(-lightDirN, normalEye);
		float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    	specular += att*specularStrength * specCoeff * lightColorP;
		
	}
}

float computeFog()
{
 vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
 float fogDensity = 0.006f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 
 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    computeDirLight();
    
	computePointLight();

    //compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	
	fColor = vec4(color, 1.0f);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	fColor = fogColor*(1-fogFactor)+fColor*fogFactor;

}