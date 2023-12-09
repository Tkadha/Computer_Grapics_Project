#version 330 core

in vec4 out_Color; 
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D outTexture;

void main(void)
{
	float ambientLight = 0.7;
	vec3 ambient = ambientLight * lightColor;

	vec3 normalVector = normalize(Normal);
	vec3 lightDir = normalize (lightPos - FragPos);
	float diffuseLight = max(dot(normalVector,lightDir),0.0);
	vec3 diffuse = diffuseLight * lightColor;

	int shininess = 128;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect (-lightDir, normalVector);
	float specularLight = max(dot(viewDir, reflectDir), 0.0);
	specularLight = pow(specularLight,shininess);
	vec3 specular = specularLight * lightColor;
	
	vec3 result = (ambient + diffuse + specular) * out_Color.xyz;

	FragColor = vec4(result, out_Color.a);
	FragColor = texture(outTexture, TexCoord) * FragColor;
}