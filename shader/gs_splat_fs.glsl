#version 330 core

precision highp float;

in vec2 vPosition;
in vec4 vColor;
in float vPositionScale;

out vec4 fragColor;

void main()
{
	float A = -dot(vPosition, vPosition);
	if (A < -vPositionScale * vPositionScale)
		discard;
	float B = exp(A) * vColor.a;
	fragColor = vec4(B * vColor.rgb, B);
}