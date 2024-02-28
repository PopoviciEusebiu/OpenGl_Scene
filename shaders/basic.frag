#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosEye;
vec3 ambient;
vec3 diffuse;
vec3 specular;
uniform vec3 pos;
uniform vec3 pos1;
uniform vec3 caseLight;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform int fogEnable;
uniform float densityC;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float ct = 1.0f;
float linear = 0.09f;
float quadratic = 0.032f;

float getFog(float density)
{	
	float fragmentDistance = length(fragPosEye);

	float ret = exp(-pow(fragmentDistance * density,2));
	return clamp(ret, 0.0f, 1.f);
}


vec3 calcAttenuation(vec3 pos)
{
	float distance    = length(pos - fPosition);
	float attenuation = 1.0 / (ct + linear * distance + 
					quadratic * (distance * distance)); 
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation; 
	
	return(ambient + diffuse + specular);
}
vec3 computePointLight(){
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
	
	vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    vec3 viewDir = normalize(- fPosEye.xyz);

    ambient = ambientStrength * lightColor;

    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
	
	return calcAttenuation(pos);
	
	
}

vec3 computeDirLight()
{
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    vec3 viewDir = normalize(- fPosEye.xyz);

    ambient = ambientStrength * lightColor;

    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
	
	return(ambient + diffuse + specular);
}


vec3 computePointLight1(){
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
	
	vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    vec3 viewDir = normalize(- fPosEye.xyz);

    ambient = ambientStrength * lightColor;

    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
	
	return calcAttenuation(pos1);
	
	
}

void main() 
{
	
	//compute fog
	vec3 fogColor = vec3(0.5f, 0.5f, 0.5f);

	vec3 dirLight = computeDirLight();
	vec3 pLight = computePointLight();
	vec3 pLight1 = computePointLight1();
	vec3 res;
	vec3 ones = vec3(1.0f, 1.0f, 1.0f);
	if(caseLight == ones){
		res = dirLight;
		}
	else{
		res = pLight + pLight1;
	}
	vec3 color = min(res * texture(diffuseTexture, fTexCoords).rgb, 1.0);
    	fColor = vec4(color, 1.0f);
	
	if (fogEnable == 1)
		fColor = mix(vec4(fogColor, 1.f), fColor, getFog(densityC));


}
