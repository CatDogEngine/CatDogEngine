$input v_color0, v_color1
#include "../common/common.sh"

void main()
{
    // A = -|v_color1|^2
	float A = -dot(v_color1.xy, v_color1.xy);
    // 如果 A 小于 -4.0，则丢弃该片段，这实际上创造了一个椭圆形状的裁剪区域。
    if (A < -4.0) discard;
    // exp(A) 在 0 -> -4 的范围内衰减得很快，这将会创造一个从中心逐渐淡出的椭圆。
    float B = exp(A) * v_color0.a;
    gl_FragColor = vec4(B * v_color0.rgb, B);
    gl_FragColor = vec4_splat(1.0);
}
