#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 LightingColor; // 传递给片段着色器

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float Shininess;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 NormalMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    //转换到世界空间
	vec3 FragPos = vec3(model * vec4(aPos, 1.0));
	//进行法向量变换
	vec3 Normal = NormalMatrix * aNormal;
    
    // ambient
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
	diff = diffuseStrength * diff;
    vec3 diffuse = diff * lightColor;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
    vec3 specular = specularStrength * spec * lightColor;      

    LightingColor = ambient + diffuse + specular;
}