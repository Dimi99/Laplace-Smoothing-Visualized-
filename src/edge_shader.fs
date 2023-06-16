#version 330 core

in vec4 Colour;
out vec4 FragColor;

void main()
{
    FragColor = Colour;
    // Set the fragment depth value
    //gl_FragDepth = 0.5;  // Set the depth value to 0.5
}