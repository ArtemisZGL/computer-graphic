#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float Shininess;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视转换
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 转换到0-1的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 根据坐标从shadowmap中获取深度值，这个就是光源能照射到的最大的深度值
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 然后获取现在这个坐标下的深度值
    float currentDepth = projCoords.z;
    // 根据法向量和光线方向向量来计算偏移值，加上偏移值之后在比较大小
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF，在3*3邻域取平均值
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    //超出视锥体的话直接假设不在阴影中
    if(projCoords.z > 1.0)
        shadow = 0.0;
    // float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;    
    return shadow;
}

void main()
{           
    vec3 color = objectColor;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = ambientStrength * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
	diffuse = diffuseStrength * diffuse;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), Shininess);
    vec3 specular = specularStrength * spec * lightColor;  
    // 增加了一个计算shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace); 
	//shadow只影响duffuse和specular
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}