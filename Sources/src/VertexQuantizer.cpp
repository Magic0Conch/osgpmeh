#include "VertexQuantizer.h"


void adaptiveQuantizeAttributes(const std::vector<Vertex>& original_vertices,
                                std::vector<Vertex>& quantized_vertices,
                                size_t vertex_size,
                                float position_levels,
                                float normal_levels,
                                float uv_levels) {
    size_t vertex_count = original_vertices.size() ;
    quantized_vertices.resize(original_vertices.size());

    // 初始化属性范围
    float position_min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
    float position_max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    float normal_min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
    float normal_max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    float uv_min[2] = {FLT_MAX, FLT_MAX};
    float uv_max[2] = {-FLT_MAX, -FLT_MAX};

    // 计算属性的最大值和最小值
    for (size_t i = 0; i < vertex_count; ++i) {
        auto& vertex = original_vertices[i];

        // 更新位置范围
        for (int j = 0; j < 3; ++j) {
            position_min[j] = std::min(position_min[j], vertex.position[j]);
            position_max[j] = std::max(position_max[j], vertex.position[j]);
        }

        // 更新UV范围
        for (int j = 0; j < 2; ++j) {
            uv_min[j] = std::min(uv_min[j], vertex.uv[j]);
            uv_max[j] = std::max(uv_max[j], vertex.uv[j]);
        }

        // 更新法线范围
        for (int j = 0; j < 3; ++j) {
            normal_min[j] = std::min(normal_min[j], vertex.normal[j]);
            normal_max[j] = std::max(normal_max[j], vertex.normal[j]);
        }
    }

    // 计算 scale
    float position_scale[3];
    float normal_scale[3];
    float uv_scale[2];

    for (int j = 0; j < 3; ++j) {
        position_scale[j] = (position_max[j] - position_min[j]) / position_levels;
        normal_scale[j] = (normal_max[j] - normal_min[j]) / normal_levels;
    }
    for (int j = 0; j < 2; ++j) {
        uv_scale[j] = (uv_max[j] - uv_min[j]) / uv_levels;
    }
    // uv_scale[0] = {4};
    // uv_scale[1] = {4};
    // 应用量化到临时缓冲区
    for (size_t i = 0; i < vertex_count; ++i) {
        Vertex* quantized_vertex = &quantized_vertices[i];        
        const Vertex* vertex = &original_vertices[i];

        // 量化位置
        for (int j = 0; j < 3; ++j) {
            quantized_vertex->position[j] =
                round((vertex->position[j] - position_min[j]) / position_scale[j]) * position_scale[j] + position_min[j];
        }

        // 量化UV
        for (int j = 0; j < 2; ++j) {
            quantized_vertex->uv[j] =
                round((vertex->uv[j] - uv_min[j]) / uv_scale[j]) * uv_scale[j] + uv_min[j];
        }

        // 量化法线
        for (int j = 0; j < 3; ++j) {
            quantized_vertex->normal[j] =
                round((vertex->normal[j] - normal_min[j]) / normal_scale[j]) * normal_scale[j] + normal_min[j];
        }
    }
}

void adaptiveQuantizeAttributesWithLevels(const std::vector<Vertex>& original_vertices,
                                          std::vector<Vertex>& quantized_vertices,
                                          size_t vertex_size,
                                          MergeLevel merge_level) {
    float position_levels = 1024;  // 默认值
    float normal_levels = 256;
    float uv_levels = 1024;

    // 根据合并等级设置 levels
    switch (merge_level) {
        case ULTRA_HIGH:
            position_levels = 8192;
            normal_levels = 2048;
            uv_levels = 8192;
            break;
        case HIGH:
            position_levels = 4096;
            normal_levels = 1024;
            uv_levels = 4096;
            break;
        case MEDIUM_HIGH:
            position_levels = 2048;
            normal_levels = 512;
            uv_levels = 2048;
            break;
        case MEDIUM:
            position_levels = 1024;
            normal_levels = 256;
            uv_levels = 1024;
            break;
        case LOW_MEDIUM:
            position_levels = 1024;
            normal_levels = 128;
            uv_levels = 512;
            break;
        case LOW:
            position_levels = 1024;
            normal_levels = 64;
            uv_levels = 256;
            break;
        case PREVIEW_HIGH:
            position_levels = 1024;
            normal_levels = 32;
            uv_levels = 128;
            break;
        case PREVIEW_MEDIUM:
            position_levels = 1024;
            normal_levels = 16;
            uv_levels = 64;
            break;
        case PREVIEW_LOW:
            position_levels = 1024;
            normal_levels = 8;
            uv_levels = 32;
            break;
        case MINIMAL:
            position_levels = 256;
            normal_levels = 4;
            uv_levels = 16;
            break;
    }

    // 调用量化逻辑
    adaptiveQuantizeAttributes(original_vertices, quantized_vertices, vertex_size,
                               position_levels, normal_levels, uv_levels);
}