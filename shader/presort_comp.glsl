#version 440 core
layout(local_size_x = 256) in;

uniform mat4 modelViewProj;
uniform vec2 nearFar;
uniform uint keyMax;

layout(binding = 4, offset = 0) uniform atomic_uint output_count;

layout(std430, binding = 0) readonly buffer PosBuffer
{
	vec4 positions[];
};

layout(std430, binding = 1) writeonly buffer OutputBuffer
{
	uint quantizedZs[];
};

layout(std430, binding = 2) writeonly buffer OutputBuffer2
{
	uint indices[];
};

void main()
{
	uint idx = gl_GlobalInvocationID.x;

	uint len = uint(positions.length());
	if (idx >= len)
	{
		return;
	}

	// NOTE: alpha is encoded into the w component of the positions
	vec4 p = modelViewProj * positions[idx];
	float depth = p.z;
	float xx = p.x / p.w;
	float yy = p.y / p.w;

	const float CLIP = 1.5f;
	//if (depth > 0.0f && xx < CLIP && xx > -CLIP && yy < CLIP && yy > -CLIP)
	//{
	uint count = atomicCounterIncrement(output_count);
	// 16.16 fixed point
	uint fixedPointZ = uint(0xffffffff) - uint(clamp(depth, 0.0f, 65535.0f) * 65536.0f);
	//uint fixedPointZ = keyMax - uint((depth / nearFar.y) * keyMax);

	quantizedZs[count] = fixedPointZ;
	indices[count] = idx;
	//}
}