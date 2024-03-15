#version 450

layout(binding = 1) uniform sampler2D u_Textures[32];

layout (location = 0) in vec4 v_Color;
layout (location = 1) in vec2  v_TexCoord;
layout (location = 2) in float v_TexIndex;

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec2 o_TexCoord;

void main() {
	vec4 texColor = v_Color;
	o_TexCoord = v_TexCoord;

	switch(int(v_TexIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], o_TexCoord); break;
		case  1: texColor *= texture(u_Textures[ 1], o_TexCoord); break;
		case  2: texColor *= texture(u_Textures[ 2], o_TexCoord); break;
		case  3: texColor *= texture(u_Textures[ 3], o_TexCoord); break;
		case  4: texColor *= texture(u_Textures[ 4], o_TexCoord); break;
		case  5: texColor *= texture(u_Textures[ 5], o_TexCoord); break;
		case  6: texColor *= texture(u_Textures[ 6], o_TexCoord); break;
		case  7: texColor *= texture(u_Textures[ 7], o_TexCoord); break;
		case  8: texColor *= texture(u_Textures[ 8], o_TexCoord); break;
		case  9: texColor *= texture(u_Textures[ 9], o_TexCoord); break;
		case 10: texColor *= texture(u_Textures[10], o_TexCoord); break;
		case 11: texColor *= texture(u_Textures[11], o_TexCoord); break;
		case 12: texColor *= texture(u_Textures[12], o_TexCoord); break;
		case 13: texColor *= texture(u_Textures[13], o_TexCoord); break;
		case 14: texColor *= texture(u_Textures[14], o_TexCoord); break;
		case 15: texColor *= texture(u_Textures[15], o_TexCoord); break;
		case 16: texColor *= texture(u_Textures[16], o_TexCoord); break;
		case 17: texColor *= texture(u_Textures[17], o_TexCoord); break;
		case 18: texColor *= texture(u_Textures[18], o_TexCoord); break;
		case 19: texColor *= texture(u_Textures[19], o_TexCoord); break;
		case 20: texColor *= texture(u_Textures[20], o_TexCoord); break;
		case 21: texColor *= texture(u_Textures[21], o_TexCoord); break;
		case 22: texColor *= texture(u_Textures[22], o_TexCoord); break;
		case 23: texColor *= texture(u_Textures[23], o_TexCoord); break;
		case 24: texColor *= texture(u_Textures[24], o_TexCoord); break;
		case 25: texColor *= texture(u_Textures[25], o_TexCoord); break;
		case 26: texColor *= texture(u_Textures[26], o_TexCoord); break;
		case 27: texColor *= texture(u_Textures[27], o_TexCoord); break;
		case 28: texColor *= texture(u_Textures[28], o_TexCoord); break;
		case 29: texColor *= texture(u_Textures[29], o_TexCoord); break;
		case 30: texColor *= texture(u_Textures[30], o_TexCoord); break;
		case 31: texColor *= texture(u_Textures[31], o_TexCoord); break;
	}

	if (texColor.a == 0.0)
		discard;

	o_Color = texColor;
}