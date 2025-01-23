#pragma once
#include <vector>
#include "Vertex.h"
enum MergeLevel {
    ULTRA_HIGH,
    HIGH,
    MEDIUM_HIGH,
    MEDIUM,
    LOW_MEDIUM,
    LOW,
    PREVIEW_HIGH,
    PREVIEW_MEDIUM,
    PREVIEW_LOW,
    MINIMAL
};


void adaptiveQuantizeAttributes(const std::vector<Vertex>& original_vertices,
                                std::vector<Vertex>& quantized_vertices,
                                size_t vertex_size,
                                float position_levels = 1024,
                                float normal_levels = 256,
                                float uv_levels = 1024);

void adaptiveQuantizeAttributesWithLevels(const std::vector<Vertex>& original_vertices,
                                          std::vector<Vertex>& quantized_vertices,
                                          size_t vertex_size,
                                          MergeLevel merge_level);