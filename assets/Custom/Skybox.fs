#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 DepthColor;

in vec4 worldPixel;

uniform int sunSize;
uniform vec3 cameraPosition;
uniform vec3 sunColor;


uniform vec3 lightDirection;

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

void main() 
{
    vec3 lightDir = normalize(lightDirection);
    lightDir.r = -lightDir.r;
    vec3 viewDirection = normalize(worldPixel.xyz - cameraPosition);
    vec3 top = vec3(68 / 255.0, 118 / 255.0, 189 / 255.0);
    vec3 bot = vec3(188 / 255.0, 214 / 255.0, 231 / 255.0);
    float sun = pow(max(dot(-viewDirection, lightDir), 0.0), sunSize);
    FragColor = vec4(lerp(bot, top, viewDirection.y) + sun * sunColor, 1.0);
    DepthColor = vec4(1);
}