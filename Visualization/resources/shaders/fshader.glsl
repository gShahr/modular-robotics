#version 330 core
in vec4 vertexColor;
in vec2 texCoord;
out vec4 FragColor;
uniform sampler2D tex;

void main()
{
    FragColor = mix(texture(tex, texCoord), vertexColor, 0.5);
}