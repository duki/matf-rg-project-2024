//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 viewPos;

// direkciono
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// tackasto
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;


void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // sunce
    vec3 lDir = normalize(-dirLightDir);
    float diffDir = max(dot(norm, lDir), 0.0);
    vec3 hDir = normalize(lDir + viewDir);
    float specDir = pow(max(dot(norm, hDir), 0.0), 64.0);

    vec3 dirAmbient = 0.10 * dirLightColor * objectColor; 
    vec3 dirDiffuse = diffDir * dirLightColor * objectColor;
    vec3 dirSpecular = 0.2 * specDir * dirLightColor;
    vec3 dirResult = dirAmbient + dirDiffuse + dirSpecular;

    // tackasto
    vec3 pDir = normalize(pointLightPos - FragPos);
    float diffPoint = max(dot(norm, pDir), 0.0);
    vec3 hPoint = normalize(pDir + viewDir);
    float specPoint = pow(max(dot(norm, hPoint), 0.0), 64.0);

    float dist = length(pointLightPos - FragPos);
    float att = 1.0 / (1.0 + 0.22 * dist + 0.20 * dist * dist);

    vec3 pointAmbient = 0.02 * pointLightColor * objectColor;
    vec3 pointDiffuse = diffPoint * pointLightColor * objectColor;
    vec3 pointSpecular = 0.3 * specPoint * pointLightColor;
    vec3 pointResult = (pointAmbient + pointDiffuse + pointSpecular) * att;

    FragColor = vec4(dirResult + pointResult, 1.0);
}
