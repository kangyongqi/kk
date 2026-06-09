#version 330 core
in vec2 v_texCoord;

uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

out vec4 FragColor;

void main() {
    // Y 分量直接采样（范围 [0, 1]）
    float y = texture(yTexture, v_texCoord).r;

    // **修正 UV 纹理坐标**：UV 纹理尺寸是 Y 的 1/2，坐标需映射到 [0, 1] 范围内的 UV 纹理
    vec2 uvCoord = v_texCoord * vec2(1.0, 1.0);  // 不缩放，直接使用原坐标，但 UV 纹理尺寸是 1/2
    // 等价于：uvCoord.x = v_texCoord.x * 0.5 + 0.0;  // 左半部分（若 UV 存储为平面左半）
    // 或根据实际存储方式调整，YUV420P 中 UV 通常是水平和垂直均为 1/2，故坐标无需缩放，直接采样即可

    float u = texture(uTexture, uvCoord).r - 0.5;  // 减去偏置（YUV 格式中 U/V 范围通常是 [-0.5, 0.5]）
    float v = texture(vTexture, uvCoord).r - 0.5;

    // BT.601 转换矩阵（适用于 4:2:0 采样，假设输入为平面 YUV）
    vec3 rgb;
    rgb.r = y + 1.402 * v;
    rgb.g = y - 0.34414 * u - 0.71414 * v;
    rgb.b = y + 1.772 * u;

    FragColor = vec4(clamp(rgb, 0.0, 1.0), 1.0);
}
