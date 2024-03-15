#version 450

layout (std140, binding = 0) uniform camera {
    mat4 u_ViewProjection;
} Camera;

layout (location = 0) in vec4  a_Position;
layout (location = 1) in vec4  a_Color;
layout (location = 2) in vec2  a_TexCoord;
layout (location = 3) in float a_TexIndex;

layout (location = 0) out vec4  v_Color;
layout (location = 1) out vec2  v_TexCoord;
layout (location = 2) out float v_TexIndex;

void main()
{
   v_Color = a_Color;
   v_TexCoord = a_TexCoord;
   v_TexIndex = a_TexIndex;

   gl_Position = Camera.u_ViewProjection * a_Position;
   gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}