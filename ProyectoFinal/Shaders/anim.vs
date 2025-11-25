#version 330 core
layout (location = 0) in vec3 aPos;       // Posición del vértice
layout (location = 1) in vec2 aTexCoords; // Coordenadas de textura

out vec2 TexCoords; // Pasar las coordenadas de textura al fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transformar la posición del vértice
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pasar las coordenadas de textura
    TexCoords = aTexCoords;
}
