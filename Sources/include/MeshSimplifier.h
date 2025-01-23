#pragma once
#include "VertexQuantizer.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstddef> 
#include <osg/Geometry>
// 合并等级

class MeshSimplifier {
public:
    MeshSimplifier() = default;
    ~MeshSimplifier() = default;

    bool loadMesh(osg::Geometry* geometry);

    // 保存简化后的网格
    bool saveMesh(std::vector<Vertex>& out_vertices, std::vector<unsigned int>& out_indices) const;

    // 设置简化等级
    void setSimplificationLevel(MergeLevel level = MINIMAL);
    void setLevels(float position_levels,float normal_levels,float uv_levels);
    void setDebugFlag(bool flag);
    // 网格简化
    bool simplify(float target_ratio, float error_tolerance = 1e-2f);

    // 统计信息
    void printConnectedComponents() const;

    std::vector<Vertex>& getVertices();
    std::vector<unsigned int>& getIndices();
    size_t getVertexCount() const;
    size_t getVertexSize() const;
private:
    // 原始数据
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    float position_levels = 1024;
    float normal_levels = 256;
    float uv_levels = 1024;

    size_t vertex_size; 
    size_t vertex_count;
    bool debug_flag = false;
    // 当前简化级别
    MergeLevel simplification_level = MINIMAL;

    mutable bool mesh_loaded = false;

    // 内部方法：量化顶点
    void quantizeAttributes(std::vector<float>& vertices, int position_levels, int normal_levels, int uv_levels);

    // 内部方法：重映射顶点和索引
    void remap(std::vector<float>& vertices, std::vector<unsigned int>& indices);

    void searchConnectedComponents(int vertex, const std::unordered_map<int, std::unordered_set<int>>& adjList, 
         std::unordered_set<int>& visited, std::unordered_set<int>& component) const;
    
    bool extractMeshData(osg::Geometry* geometry,
                    std::vector<Vertex>& vertices,
                    std::vector<unsigned int>& indices);

    bool extractIndices(osg::PrimitiveSet* primitiveSet, std::vector<unsigned int>& indices);

    void remap(const std::vector<Vertex>& original_vertices,
           std::vector<Vertex>& vertices,
           std::vector<unsigned int>& indices);

    void simplifyMeshWithAttributes(std::vector<float>& vertices,
            std::vector<unsigned int>& indices,
            size_t target_index_count);

    void simplifyMeshWithAttributes(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        size_t target_index_count,
        float target_error
    );
};
