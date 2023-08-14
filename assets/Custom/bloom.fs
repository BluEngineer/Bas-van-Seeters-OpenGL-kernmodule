#version 330 core

out vec4 FragColor;
in vec2 fragCoord;

uniform sampler2D _MainTex;
uniform vec2 screenResolution;

const float offset_x = 1.0f / 1700.0f;  
const float offset_y = 1.0f / 900.0f;  

vec2 offsets[9] = vec2[]
(
    vec2(-offset_x,  offset_y), vec2( 0.0f,    offset_y), vec2( offset_x,  offset_y),
    vec2(-offset_x,  0.0f),     vec2( 0.0f,    0.0f),     vec2( offset_x,  0.0f),
    vec2(-offset_x, -offset_y), vec2( 0.0f,   -offset_y), vec2( offset_x, -offset_y) 
);

float kernel[9] = float[]
(
    -5, 1, 1,
    -1, 3, -1,
    9, -1, -6
);

void main()
{
    vec2 position = fragCoord - vec2(0.5);
    float dist = length(position);
    float vignette = smoothstep(0.85, 0.3, dist);

    vec4 color = texture(_MainTex, fragCoord);
    for(int i = 0; i < 9; i++)
    {
        color.rgb += vec3(texture(_MainTex, fragCoord.st + offsets[i])) * kernel[i];
    }

    // Output the final color with vignette effect
    FragColor = vec4(color.rgb * vignette, color.a);
}