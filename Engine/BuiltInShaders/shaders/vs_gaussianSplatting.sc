
$input a_position
$output v_color0, v_color1

#include "../common/common.sh"

USAMPLER2D(u_texture, 0);

uniform mat4 projection;
uniform mat4 view;
uniform vec4 focal; // vec2
uniform vec4 viewport; // vec2
uniform vec4 depthIndex;

vec2 Unpack(uint x)
{
	return vec2(uintBitsToFloat((x & 0x0000ffff) << 16), uintBitsToFloat(x & 0xffff0000));
}

void main()
{
	// 这里本来应该用实例化的 index 即 i_data0.x
	uint index = floatBitsToUint(depthIndex.x);
	// 从纹理 u_texture 中获取中心点数据 cen，使用 index 计算纹理坐标。
	uvec4 cen = texelFetch(u_texture, ivec2((index & 0x3ffu) << 1, index >> 10), 0);
	// 将中心点数据转换为浮点数，并应用视图和投影变换，得到 2D 位置 pos2d。
	vec4 cam = mul(view, vec4(uintBitsToFloat(cen.xyz), 1.0));
	vec4 pos2d = mul(projection, cam);

	// 计算裁剪范围 clip，并检查 pos2d 是否在裁剪范围内。
	// 如果不在范围内，将顶点位置设置为 (0.0, 0.0, 2.0, 1.0) 并返回。
	float clip = 1.2 * pos2d.w;
	if(pos2d.x < -clip || pos2d.y < -clip || pos2d.z < -clip ||
		pos2d.x > clip || pos2d.y > clip)
	{
		gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
	}
	else
	{
		// 从纹理 u_texture 中获取协方差数据 cov，并解包为半精度浮点数，构建 3x3 矩阵 Vrk。
		uvec4 cov = texelFetch(u_texture, ivec2(((index & 0x3ffu) << 1) | 1u, index >> 10), 0);
		vec2 u1 = Unpack(cov.x);
		vec2 u2 = Unpack(cov.y);
		vec2 u3 = Unpack(cov.z);
		mat3 Vrk = mat3(u1.x, u1.y, u2.x, u1.y, u2.y, u3.x, u2.x, u3.x, u3.y);
		
		// 构建雅可比矩阵 J，用于将 3D 空间中的点转换为 2D 图像平面上的点。
		// https://math.stackexchange.com/questions/4716499/pinhole-camera-projection-of-3d-multivariate-gaussian
		mat3 J = mat3(
			focal.x / cam.z, 0.0, -(focal.x * cam.x) / (cam.z * cam.z), 
			0.0, -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z), 
			0.0, 0.0, 0.0
		);

		// 计算变换矩阵 T，并将协方差矩阵 Vrk 转换为 2D 协方差矩阵 cov2d。
		// 用于将 3D 空间中的点转换为 2D 图像平面上的点，
		// 并计算这些点在 2D 平面上的不确定性（即误差椭圆）。
		mat3 T = transpose((mat3)u_view) * J;
		mat3 cov2d = transpose(T) * Vrk * T;

		// 计算协方差矩阵 cov2d 的特征值 lambda1 和 lambda2。
		float mid = (cov2d[0][0] + cov2d[1][1]) / 2.0;
		float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) / 2.0, cov2d[0][1]));
		float lambda1 = mid + radius;
		float lambda2 = mid - radius;

		if(lambda2 >= 0.0)
		{
			// 计算主轴和次轴，用于绘制椭圆。
			vec2 diagonalVector = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));
			vec2 majorAxis = min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;
			vec2 minorAxis = min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);

			// 计算顶点颜色 vColor，并将顶点位置 position 传递给片段着色器。
			v_color0 = clamp(pos2d.z / pos2d.w + 1.0, 0.0, 1.0) *
				vec4(cov.w & 0xffu,
					(cov.w >> 8) & 0xffu,
					(cov.w >> 16) & 0xffu,
					(cov.w >> 24) & 0xffu
					) / 255.0;
			v_color1.xy = a_position.xy;

			// 计算顶点在屏幕上的位置 gl_Position，并应用主轴和次轴的变换。
			vec2 vCenter = pos2d.xy / pos2d.w;
			vec2 pos = vCenter +
				a_position.x * majorAxis / viewport.xy +
				a_position.y * minorAxis / viewport.xy;
			gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
		}
	}
}
