#version 330 core
out vec4 FragColor;

in vec3 Pos;

uniform vec4 ourColor; // we set this variable in the OpenGL code.
uniform sampler2D tfTex; // transfer function texture
uniform sampler3D ourTexture;
uniform bool textureMapFlag;

// variables used to calculate the transfer function
uniform int transfer_func_range;
uniform int slide_space;
uniform float slider0;
uniform float slider1;
uniform float slider2;
uniform float slider3;
uniform float slider4;
uniform float slider5;

// declare method
float compute_transfer_func(int x, int range, int slide_space, float slider0, float slider1, float slider2, float slider3, float slider4, float slider5);

void main()
{
    if (textureMapFlag) {
        vec3 TexCoord = vec3(1.0, 1.0, 1.0) - Pos;
        float intensity = texture(ourTexture, TexCoord).x;
        int xPos = int(intensity * 256);
        vec2 tfTexCoord = vec2(intensity, 0.0);
        vec4 tf = texture(tfTex, tfTexCoord);
        float tfVal = compute_transfer_func(xPos, transfer_func_range, slide_space, slider0, slider1, slider2, slider3, slider4, slider5);
        FragColor = vec4(tf.rgb*tfVal, tfVal);
    }
    else {
        FragColor = ourColor;
    }
}

float compute_transfer_func(int x, int range, int slide_space, float slider0, float slider1, float slider2, float slider3, float slider4, float slider5)
{
    int pos = int(x / slide_space);
    float slider_before;
    float slider_after;
    if (pos == 0) {
        slider_before = slider0;
        slider_after = slider1;
    }
    else if (pos == 1) {
        slider_before = slider1;
        slider_after = slider2;
    }
    else if (pos == 2) {
        slider_before = slider2;
        slider_after = slider3;
    }
    else if (pos == 3) {
        slider_before = slider3;
        slider_after = slider4;
    }
    else if (pos == 4) {
        slider_before = slider4;
        slider_after = slider5;
    }
    int slider_before_x = pos * slide_space;
    int slider_after_x = (pos + 1) * slide_space;
    float v = slider_before + (slider_after-slider_before) * (float((x-slider_before_x)) / (slider_after_x-slider_before_x));
    //func[i] = func[slider0_x] + (func[slider1_x] - func[slider0_x]) * ((float(i) - slider0_x) / (slider1_x - slider0_x));
    return v;
}


