#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    // calculating the normalt matrix is expensive
    // and should probably be done on the cpu and send to a uniform
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    //Normal = aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}