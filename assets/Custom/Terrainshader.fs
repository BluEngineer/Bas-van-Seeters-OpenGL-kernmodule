#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 DepthColor;

in float lightIntensity;
in vec3 lightColor;
in vec3 color;
in vec2 uv;
in vec3 normal;
in vec4 worldPixel;

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform sampler2D dirt, sand, grass, rock, snow;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform float waterHeight;
uniform int clipDir;

float dirtTiling = 2.0;
float sandTiling = 5.0;
float grassTiling = 25.0;
float rockTiling = 10.0;
float snowTiling = 70.0;

vec3 lerp(vec3 a, vec3 b, float t)
{
    return a + (b - a) * t;
}

void clip(float d)
{
    if (d < 0) discard;
}

void main() 
{
    clip(clipDir * (waterHeight - worldPixel.y));

    vec3 normalColor = texture(normalmap, uv).rbg * 2 - 1;
    normalColor.b = -normalColor.b;

    vec3 lightDir = normalize(lightDirection);
    float light = max(dot(-lightDir, normalColor), .20);

    vec2 scaledUV = uv * vec2(dirtTiling, dirtTiling);
    vec3 dirtColor = texture(dirt, scaledUV).rgb;

    scaledUV = uv * vec2(sandTiling, sandTiling);
    vec3 sandColor = texture(sand, scaledUV).rgb;

    scaledUV = uv * vec2(grassTiling, grassTiling);
    vec3 grassColor = texture(grass, scaledUV).rgb;

    scaledUV = uv * vec2(rockTiling, rockTiling);
    vec3 rockColor = texture(rock, scaledUV).rgb;

    scaledUV = uv * vec2(snowTiling, snowTiling);
    vec3 snowColor = texture(snow, scaledUV).rgb;

    float ds = clamp((worldPixel.y - 1f) / 10, 0, 1);
    float sg = clamp((worldPixel.y - 8f) / 10, 0, 1);
    float gr = clamp((worldPixel.y - 20) / 10, 0, 1);
    float rs = clamp((worldPixel.y -38) / 10, 0, 1);

    vec3 diffuse = lerp(lerp(lerp(lerp(dirtColor, sandColor, ds), grassColor, sg), rockColor, gr), snowColor, rs);
    float d = distance(worldPixel.xyz, cameraPosition);
    float fogAmount = clamp((d - 250) / 250, 0, 1);

    vec3 bot = vec3(188 / 255.0, 214 / 255.0, 231 / 255.0);

    FragColor = vec4(lerp(diffuse * light, bot, fogAmount), 1.0);
    DepthColor = vec4(d / 100);
}