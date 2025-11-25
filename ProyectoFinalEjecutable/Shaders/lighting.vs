#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const float PI = 3.14159;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out float trans;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int anim;
uniform float transparencia;

void main()
{
    //Animación
    if(anim==1){
        float angle = 20.0 * sin(time * 3.0); // Cambia 0.5 para controlar la velocidad de oscilación

    // Convierte el ángulo a radianes
    float radians = radians(angle);

    // Calcula la matriz de rotación usando el ángulo en radianes
    float cosAngle = cos(radians);
    float sinAngle = sin(radians);
    mat2 rotation = mat2(cosAngle, -sinAngle, 
                         sinAngle, cosAngle);
    
    // Aplica la rotación a las coordenadas de textura y las centra en (0.5, 0.5)
    TexCoords = rotation * (aTexCoords - vec2(0.5)) + vec2(0.5);
    }
    else{
    TexCoords = aTexCoords;
    }
    
    // Transformación de la posición del vértice
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Calcula la posición del fragmento en el espacio mundial
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calcula la normal transformada
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Asigna el valor de transparencia
    trans = transparencia;
}
