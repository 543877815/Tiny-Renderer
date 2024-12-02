#version 330 core
precision highp float;
uniform vec4 color;
uniform sampler2D colorTexture;
in vec2 frag_uv;
out vec4 fragColor;

void main(void)
{
	//#ifdef USE_SUPERSAMPLING
	//	// per pixel screen space partial derivatives
	//	vec2 dx = dFdx(frag_uv) * 0.25; // horizontal offset
	//	vec2 dy = dFdy(frag_uv) * 0.25; // vertical offset
	//
	//	// supersampled 2x2 ordered grid
	//	vec4 texColor = vec4(0);
	//	texColor += texture(colorTexture, vec2(frag_uv + dx + dy));
	//	texColor += texture(colorTexture, vec2(frag_uv - dx + dy));
	//	texColor += texture(colorTexture, vec2(frag_uv + dx - dy));
	//	texColor += texture(colorTexture, vec2(frag_uv - dx - dy));
	//	texColor *= 0.25;
	//#else
	vec4 texColor = texture(colorTexture, frag_uv);
	//#endif

	fragColor = texColor;
}
