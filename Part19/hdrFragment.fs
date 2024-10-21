#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D brightTexture;

void main(){
    vec3 hdrColor = vec3(texture(texture1, TexCoord));
    vec3 bloomColor = vec3(texture(brightTexture, TexCoord));
    hdrColor += bloomColor;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * 1.0);

    FragColor = vec4(mapped, 1.0);

}
