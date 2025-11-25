#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform float time;

void main()
{
    // Genera un patrón de ruido pseudoaleatorio (velocidad reducida a 1/3 de la original)
    float noise = fract(sin(dot(TexCoords.xy * 120.0 + time * 2.0, vec2(12.9898, 78.233))) * 43758.5453);
    
    // Aplica una función de potencia para mantener el contraste
    noise = pow(noise, 0.5);
    
    // Color morado fosforescente
    vec3 staticColor = vec3(
        noise * 0.9,      // Canal Rojo (90% intensidad)
        noise * 0.2,      // Canal Verde (20% intensidad)
        noise * 1.2       // Canal Azul (120% intensidad)
    );
    
    // Refuerzo del efecto fosforescente
    staticColor = mix(staticColor, staticColor * 1.5, 0.7);
    
    // Resultado final
    FragColor = vec4(staticColor, 1.0);
}