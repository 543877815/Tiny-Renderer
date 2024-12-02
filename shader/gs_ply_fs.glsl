#version 330 core
precision highp float;
in vec2 vPosition;
in vec4 vColor;
in mat2 vCov2d_inv;
in vec4 vConic_opacity;
in vec2 vCenter;
in vec4 vConic;
uniform int showGaussian;
uniform int showHotspots;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 hotspots;

void main()
{
	float minAlpha = 1.0f / 256.0f;
	vec2 d = (vCenter - gl_FragCoord.xy);
	float g = exp(-0.5f * dot(d, vCov2d_inv * d));
	if (vColor.a * g <= minAlpha)
	{
		discard;
	}

	if (showGaussian == 1)
	{
		fragColor = vec4(vColor);
	}
	else {
		fragColor = vec4(vColor.rgb * vColor.a * g, vColor.a * g);
	}
}