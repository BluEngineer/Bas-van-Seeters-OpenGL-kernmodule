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

const float saturationAmount = 5.0f; // Adjust this value to control the amount of saturation
const float hueShiftAmount = 0.6f; // Adjust this value to control the amount of hue shift
const float contrastAmount = 3f; // Adjust this value to control the contrast of the final image

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

    // Render the scene to a separate texture, excluding bright areas
    vec3 sceneColor = color.rgb;
    float brightnessThreshold = 0.8f; // Adjust this threshold to control which areas will bloom
    vec3 bloomMask = step(brightnessThreshold, sceneColor);

    // Apply a blur effect to the thresholded texture
    vec3 blurredTexture = vec3(0.0f);
    float blurIntensity = 0.02f; // Adjust this intensity to control the strength of the blur effect
    float blurSize = 2.0f; // Adjust this size to control the size of the blur kernel

    for (int i = -5; i <= 5; i++) {
        for (int j = -5; j <= 5; j++) {
            vec2 offset = vec2(float(i), float(j)) * blurSize;
            blurredTexture += vec3(texture(_MainTex, fragCoord.st + offset));
        }
    }

    blurredTexture /= 121.0f; // Divide by the number of samples to normalize the result

    // Combine the blurred texture with the original scene texture
    float bloomIntensity = 0.3f; // Adjust this intensity to control the strength of the bloom effect
    vec3 finalColor = sceneColor + blurredTexture * bloomIntensity;

    // Apply saturation effect
    vec3 gray = vec3(dot(finalColor.rgb, vec3(0.2126, 0.7152, 0.0722)));
    finalColor = mix(gray, finalColor, saturationAmount);

    // Apply hue shift effect
    finalColor = clamp(finalColor + hueShiftAmount, 0.0, 1.0);

    // Apply contrast effect
    finalColor = (finalColor - 0.5) * contrastAmount + 0.5;

    // Output the final color
    FragColor = vec4(finalColor * vignette, color.a);
}