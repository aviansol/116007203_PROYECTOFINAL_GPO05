#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture1;
uniform float time;

void main()
{
    float r = texture(texture1, TexCoords).r;
    float g = texture(texture1, TexCoords).g;
    float b = texture(texture1, TexCoords).b;
    // Efecto ondulado basado en tiempo
    float wave = 0.05 * sin(10.0 * TexCoords.y + time) + 0.05 * cos(10.0 * TexCoords.x - time);
    vec2 waveTexCoords = TexCoords + vec2(wave, wave);

    // Obtén el color de la textura desplazada
    vec4 texColor = vec4(r,g,b,texture(texture1, waveTexCoords).rgba);
    // Descarta fragmentos transparentes
    if (texColor.a < 0.1)
        discard;

    FragColor = texColor;
}
