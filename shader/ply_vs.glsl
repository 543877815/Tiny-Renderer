#version 330 core

precision highp float;
precision highp int;

layout(location = 0) in vec2 position;
layout(location = 1) in float index;

//#ifdef USE_GPU_SORT
//layout(std430, binding = 0) buffer MySSBO {
//	uint index[];
//}
//#else
//layout(location = 1) in float index;
//#endif

uniform highp usampler2D u_texture;
uniform mat4 view, projection;
uniform vec2 focal;
uniform vec2 viewport;
uniform vec3 camPos;
uniform vec2 tanFov;
uniform vec2 projParams;
uniform int sphericalHarmonicsDegree;
uniform int showGaussian;

out vec2 vPosition;
out vec2 vCenter;
out vec4 vColor;
out mat2 vCov2d_inv;

float SH_C0 = 0.28209479177387814f;
float SH_C1 = 0.4886025119029199f;
float SH_C2_0 = 1.0925484305920792f;
float SH_C2_1 = -1.0925484305920792f;
float SH_C2_2 = 0.31539156525252005f;
float SH_C2_3 = -1.0925484305920792f;
float SH_C2_4 = 0.5462742152960396f;
float SH_C3_0 = -0.5900435899266435f;
float SH_C3_1 = 2.890611442640554f;
float SH_C3_2 = -0.4570457994644658f;
float SH_C3_3 = 0.3731763325901154f;
float SH_C3_4 = -0.4570457994644658f;
float SH_C3_5 = 1.445305721320277f;
float SH_C3_6 = -0.5900435899266435f;

vec3 getDeg0(uint depthIndex)
{
	uvec4 u_shs0 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 3u, depthIndex >> 7), 0);
	vec3 result = uintBitsToFloat(u_shs0.xyz);
	return result;
}

vec3 getDeg1(vec3 dir, uint depthIndex)
{
	uvec4 u_shs0 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 3u, depthIndex >> 7), 0);
	uvec4 u_shs1 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 4u, depthIndex >> 7), 0);
	uvec4 u_shs2 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 5u, depthIndex >> 7), 0);

	vec3 sh1 = uintBitsToFloat(uvec3(u_shs0.w, u_shs1.xy));
	vec3 sh2 = uintBitsToFloat(uvec3(u_shs1.zw, u_shs2.x));
	vec3 sh3 = uintBitsToFloat(uvec3(u_shs2.yzw));

	float x = dir.x;
	float y = dir.y;
	float z = dir.z;

	vec3 result = -SH_C1 * y * sh1 + SH_C1 * z * sh2 - SH_C1 * x * sh3;
	return result;
}

vec3 getDeg2(vec3 dir, uint depthIndex)
{
	uvec4 u_shs3 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 6u, depthIndex >> 7), 0);
	uvec4 u_shs4 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 7u, depthIndex >> 7), 0);
	uvec4 u_shs5 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 8u, depthIndex >> 7), 0);
	uvec4 u_shs6 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 9u, depthIndex >> 7), 0);

	vec3 sh4 = uintBitsToFloat(uvec3(u_shs3.xyz));
	vec3 sh5 = uintBitsToFloat(uvec3(u_shs3.w, u_shs4.xy));
	vec3 sh6 = uintBitsToFloat(uvec3(u_shs4.zw, u_shs5.x));
	vec3 sh7 = uintBitsToFloat(uvec3(u_shs5.yzw));
	vec3 sh8 = uintBitsToFloat(uvec3(u_shs6.xyz));

	float x = dir.x;
	float y = dir.y;
	float z = dir.z;

	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, yz = y * z, xz = x * z;

	vec3 result = SH_C2_0 * xy * sh4 +
		SH_C2_1 * yz * sh5 +
		SH_C2_2 * (2.0f * zz - xx - yy) * sh6 +
		SH_C2_3 * xz * sh7 + SH_C2_4 * (xx - yy) * sh8;
	return result;
}

vec3 getDeg3(vec3 dir, uint depthIndex)
{
	uvec4 u_shs6 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 9u, depthIndex >> 7), 0);
	uvec4 u_shs7 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 10u, depthIndex >> 7), 0);
	uvec4 u_shs8 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 11u, depthIndex >> 7), 0);
	uvec4 u_shs9 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 12u, depthIndex >> 7), 0);
	uvec4 u_shs10 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 13u, depthIndex >> 7), 0);
	uvec4 u_shs11 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 14u, depthIndex >> 7), 0);
	uvec4 u_shs12 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 15u, depthIndex >> 7), 0);


	vec3 sh9 = uintBitsToFloat(uvec3(u_shs6.w, u_shs7.xy));
	vec3 sh10 = uintBitsToFloat(uvec3(u_shs7.zw, u_shs8.x));
	vec3 sh11 = uintBitsToFloat(uvec3(u_shs8.yzw));
	vec3 sh12 = uintBitsToFloat(uvec3(u_shs9.xyz));
	vec3 sh13 = uintBitsToFloat(uvec3(u_shs9.w, u_shs10.xy));
	vec3 sh14 = uintBitsToFloat(uvec3(u_shs10.zw, u_shs11.x));
	vec3 sh15 = uintBitsToFloat(uvec3(u_shs11.yzw));
	vec3 sh16 = uintBitsToFloat(uvec3(u_shs12.xyz));

	float x = dir.x;
	float y = dir.y;
	float z = dir.z;

	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, yz = y * z, xz = x * z;

	vec3 result = SH_C3_0 * y * (3.0f * xx - yy) * sh9 +
		SH_C3_1 * xy * z * sh10 +
		SH_C3_2 * y * (4.0f * zz - xx - yy) * sh11 +
		SH_C3_3 * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * sh12 +
		SH_C3_4 * x * (4.0f * zz - xx - yy) * sh13 +
		SH_C3_5 * z * (xx - yy) * sh14 +
		SH_C3_6 * x * (xx - 3.0f * yy) * sh15;
	return result;
}

mat2 computeCov2D(vec4 cam, uvec4 cov3d1, uvec4 cov3d2)
{
	vec4 cov3d1f = uintBitsToFloat(cov3d1);
	vec4 cov3d2f = uintBitsToFloat(cov3d2);

	float WIDTH = viewport.x;
	float HEIGHT = viewport.y;
	float Z_NEAR = projParams.x;
	float Z_FAR = projParams.y;

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

	vec2 u1 = cov3d1f.xy, u2 = cov3d1f.zw, u3 = cov3d2f.xy;
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

//mat2 computeCov2D(vec4 cam, uvec4 cov3d1, uvec4 cov3d2)
//{
//	vec4 cov3d1f = uintBitsToFloat(cov3d1);
//	vec4 cov3d2f = uintBitsToFloat(cov3d2);
//
//	mat3 J = mat3(
//		focal.x / cam.z, 0.0f, -(focal.x * cam.x) / (cam.z * cam.z),
//		0.0f, -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z),
//		0.0f, 0.0f, 0.0f
//	);
//
//	mat3 T = transpose(mat3(view)) * J;
//	vec2 u1 = cov3d1f.xy, u2 = cov3d1f.zw, u3 = cov3d2f.xy;
//	mat3 Vrk = mat3(u1.x, u1.y, u2.x,
//		u1.y, u2.y, u3.x,
//		u2.x, u3.x, u3.y);
//
//	mat3 V_prime = transpose(T) * Vrk * T;
//	mat2 cov2d = mat2(V_prime);
//	cov2d[0][0] = cov2d[0][0] + 0.3f;
//	cov2d[1][1] = cov2d[1][1] + 0.3f;
//	return cov2d;
//}

mat2 inverseMat2(mat2 m, float det)
{
	mat2 inv;
	inv[0][0] = m[1][1] / det;
	inv[0][1] = -m[0][1] / det;
	inv[1][0] = -m[1][0] / det;
	inv[1][1] = m[0][0] / det;
	return inv;
}

void main()
{
	vColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vPosition = vec2(0.0f, 0.0f);
	vCov2d_inv = mat2(1.0f, 0.0f, 0.0f, 1.0f);
	vCenter = vec2(100.0f, 100.0f);

#ifdef USE_GPU_SORT
	uint depthIndex = uint(index[gl_InstanceID]);
#else
	uint depthIndex = uint(index);
#endif

	uvec4 cen = texelFetch(u_texture, ivec2((depthIndex & 0x7fu) << 4, depthIndex >> 7), 0);
	vec3 pos3d = uintBitsToFloat(cen.xyz);
	vec4 cam = view * vec4(pos3d, 1);
	vec4 pos2d = projection * cam;
	vec3 center = vec3(pos2d) / pos2d.w;

	float limx = 1.3f * tanFov.x;
	float limy = 1.3f * tanFov.y;
	float txtz = cam.x / cam.z;
	float tytz = cam.y / cam.z;

	cam.x = min(limx, max(-limx, txtz)) * cam.z;
	cam.y = min(limy, max(-limy, tytz)) * cam.z;

	uvec4 cov3d1_4 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 1u, depthIndex >> 7), 0);
	uvec4 cov3d5_6 = texelFetch(u_texture, ivec2(((depthIndex & 0x7fu) << 4) | 2u, depthIndex >> 7), 0);

	mat2 cov2d = computeCov2D(cam, cov3d1_4, cov3d5_6);
	float det = (cov2d[0][0] * cov2d[1][1] - cov2d[0][1] * cov2d[1][0]);
	float det_inv = 1.0f / det;

	vec3 conic = vec3(cov2d[1][1] * det_inv, -cov2d[0][1] * det_inv, cov2d[0][0] * det_inv);
	float maxScreenSpaceSplatSize = 1024.0f;
	float mid = (cov2d[0][0] + cov2d[1][1]) * 0.5f;
	float term = sqrt(max(0.1f, mid * mid - det));
	float lambda1 = mid + term;
	float lambda2 = mid - term;
	if (lambda2 < 0.0f || lambda1 < 0.0f)
	{
		vColor.a = 0.0f;
		return;
	}

	vec2 eigenVector0 = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));
	vec2 eigenVector1 = normalize(vec2(eigenVector0.y, -eigenVector0.x));

	vec2 majorAxis = min(3.0f * sqrt(lambda1), maxScreenSpaceSplatSize) * eigenVector0;
	vec2 minorAxis = min(3.0f * sqrt(lambda2), maxScreenSpaceSplatSize) * eigenVector1;

	if (det == 0.0f || cov2d[0][0] < 0.0f || cov2d[1][1] < 0.0f)
	{
		vColor.a = 0.0f;
		return;
	}

	vec3 dir = pos3d - camPos;
	dir = normalize(dir);

	vec3 result = getDeg0(depthIndex);
	if (sphericalHarmonicsDegree > 0)
	{
		result += getDeg1(dir, depthIndex);
	}
	if (sphericalHarmonicsDegree > 1)
	{
		result += getDeg2(dir, depthIndex);
	}
	if (sphericalHarmonicsDegree > 2)
	{
		result += getDeg3(dir, depthIndex);
	}

	result += 0.5f;
	result = min(result, vec3(1.0f, 1.0f, 1.0f));
	result = max(result, vec3(0.0f, 0.0f, 0.0f));

	float opacity_lp = float((cov3d5_6.z >> 24) & 0xffu);
	float opacity_hp = uintBitsToFloat(cov3d5_6.w);

	vec2 offset = position.x * majorAxis + position.y * minorAxis;
	float w = pos2d.w;
	offset.x *= (2.0f / viewport.x) * w;
	offset.y *= (2.0f / viewport.y) * w;

	if (showGaussian == 1)
	{
		gl_Position = pos2d;
	}
	else
	{
		gl_Position = pos2d + vec4(offset.x, offset.y, 0.0f, 0.0f);
	}

	center.x = 0.5f * (viewport.x + center.x * viewport.x);
	center.y = 0.5f * (viewport.y + center.y * viewport.y);

	vColor = clamp(pos2d.z / pos2d.w + 1.0f, 0.0f, 1.0f) * vec4(result, opacity_hp);
	vCov2d_inv = inverseMat2(cov2d, det);
	vCenter = center.xy;
	vPosition = position;
}