$input v_color0, v_texcoord0
#include "../common/common.sh"

void main()
{
    // float A = -dot(vPosition, vPosition);：计算vPosition与自身的负点积。实际上是- (x^2 + y^2)，其中vPosition是(x, y)。
	float A = -dot(v_texcoord0, v_texcoord0);
    //if (A < -4.0) discard;：如果A小于-4.0，则丢弃该片段。这实际上创建了一个半径为2单位的圆形裁剪区域。
    if (A < -4.0) discard;
    //float B = exp(A) * vColor.a;：计算B为A的指数函数乘以vColor的alpha分量。指数函数exp(A)对于负值的A衰减得很快。
    float B = exp(A) * v_color0.a;
    //fragColor = vec4(B * vColor.rgb, B);：设置输出颜色fragColor。RGB分量按B缩放，alpha分量设置为B。
    gl_FragColor = vec4(B * v_color0.rgb, B);
}