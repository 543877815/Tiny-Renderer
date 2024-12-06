
#version 330 core
precision highp float;
precision highp int;

uniform highp usampler2D u_texture;
uniform mat4 view, projection;
uniform vec2 focal;
uniform vec2 viewport, nearFar;

layout(location = 0) in vec2 position;
//#ifdef USE_GPU_SORT
//layout(std430, binding = 0) buffer MySSBO {
//	uint index[];
//#else
layout(location = 1) in float index;
//#endif

out vec2 vPosition;
out vec4 vColor;
out float vPositionScale;

//mat2 computeCov2D(vec4 cam, uvec4 cov)
//{
//	vec2 u1 = unpackHalf2x16(cov.x), u2 = unpackHalf2x16(cov.y), u3 = unpackHalf2x16(cov.z);
//
//	mat3 Vrk = mat3(u1.x, u1.y, u2.x,
//		u1.y, u2.y, u3.x,
//		u2.x, u3.x, u3.y);
//
//	mat3 J = mat3(
//		focal.x / cam.z, 0.0f, -(focal.x * cam.x) / (cam.z * cam.z),
//		0.0f, -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z),
//		0.0f, 0.0f, 0.0f
//	);
//
//	mat3 T = transpose(mat3(view)) * J;
//
//	mat3 V_prime = transpose(T) * Vrk * T;
//	mat2 cov2d = mat2(V_prime);
//	cov2d[0][0] = cov2d[0][0] + 0.3f;
//	cov2d[1][1] = cov2d[1][1] + 0.3f;
//	return cov2d;
//}

mat2 computeCov2D(vec4 cam, uvec4 cov)
{
	vec2 u1 = unpackHalf2x16(cov.x), u2 = unpackHalf2x16(cov.y), u3 = unpackHalf2x16(cov.z);

	float WIDTH = viewport.x;
	float HEIGHT = viewport.y;
	float Z_NEAR = nearFar.x;
	float Z_FAR = nearFar.y;

	float SX = projection[0][0];
	float SY = projection[1][1];
	float WZ = projection[3][2];
	float tzSq = cam.z * cam.z;
	float jsx = -(SX * WIDTH) / (2.0f * cam.z);
	float jsy = -(SY * HEIGHT) / (2.0f * cam.z);
	float jtx = (SX * cam.x * WIDTH) / (2.0f * tzSq);
	float jty = (SY * cam.y * HEIGHT) / (2.0f * tzSq);
	float jtz = ((Z_FAR - Z_NEAR) * WZ) / (2.0f * tzSq);
	mat3 J = mat3(vec3(jsx, 0.0f, jtx),
		vec3(0.0f, jsy, jty),
		vec3(0.0f, 0.0f, jtz));

	mat3 Vrk = mat3(u1.x, u1.y, u2.x,
		u1.y, u2.y, u3.x,
		u2.x, u3.x, u3.y);

	mat3 T = transpose(mat3(view)) * J;
	mat3 V_prime = transpose(T) * Vrk * T;
	mat2 cov2d = mat2(V_prime);
	cov2d[0][0] += 0.3f;
	cov2d[1][1] += 0.3f;
	return cov2d;
}


void main()
{
	vColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vPosition = vec2(0.0f, 0.0f);

	//#ifdef USE_GPU_SORT
	//	uint depthIndex = uint(index[gl_instanceID]);
	//#else
	uint depthIndex = uint(index);
	//#endif
	uvec4 cen = texelFetch(u_texture, ivec2((uint(depthIndex) & 0x3ffu) << 1, depthIndex >> 10), 0);
	vec4 cam = view * vec4(uintBitsToFloat(cen.xyz), 1);
	vec4 pos2d = projection * cam;

	float clip = 1.2f * pos2d.w;
	if (pos2d.z < -clip || pos2d.x < -clip || pos2d.x > clip || pos2d.y < -clip || pos2d.y > clip)
	{
		gl_Position = vec4(0.0f, 0.0f, 2.0f, 1.0f);
		return;
	}

	uvec4 cov = texelFetch(u_texture, ivec2(((uint(depthIndex) & 0x3ffu) << 1) | 1u, depthIndex >> 10), 0);
	mat2 cov2d = computeCov2D(cam, cov);
	float maxScreenSpaceSplatSize = 1024.0f;
	float mid = (cov2d[0][0] + cov2d[1][1]) * 0.5f;
	float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) * 0.5f, cov2d[0][1]));
	float lambda1 = mid + radius;
	float lambda2 = mid - radius;
	if (lambda2 < 0.0f || lambda1 < 0.0f)
	{
		vColor.a = 0.0f;
		return;
	}
	vec2 eigenVector0 = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));
	vec2 eigenVector1 = normalize(vec2(eigenVector0.y, -eigenVector0.x));
	vec2 majorAxis = min(3.0f * sqrt(lambda1), maxScreenSpaceSplatSize) * eigenVector0;
	vec2 minorAxis = min(3.0f * sqrt(lambda2), maxScreenSpaceSplatSize) * eigenVector1;

	vColor = clamp(pos2d.z / pos2d.w + 1.0f, 0.0f, 1.0f)
		* vec4((cov.w) & 0xffu, (cov.w >> 8) & 0xffu, (cov.w >> 16) & 0xffu, (cov.w >> 24) & 0xffu) / 255.0f;
	vPositionScale = 2.0f;
	vPosition = position * vPositionScale;
	vec2 vCenter = vec2(pos2d) / pos2d.w;
	gl_Position = vec4(
		vCenter + vPosition.x * majorAxis / viewport + vPosition.y * minorAxis / viewport, 0.0f, 1.0f
	);
}