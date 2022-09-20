#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec3 cloud_color;
uniform vec3 lightPos = vec3(0,0,0);
uniform vec3 viewPos;

void main()
{
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    if (dot(circCoord, circCoord) > 1.0) {
        discard;
    }

    FragColor = vec4(cloud_color, 1.0);
}