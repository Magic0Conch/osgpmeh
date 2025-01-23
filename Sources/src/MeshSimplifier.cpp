#include "MeshSimplifier.h"
#include <cstddef>
#include <stack>
#include <iostream>
#include <meshoptimizer.h>
#include <vector>

void MeshSimplifier::setDebugFlag(bool flag){
    debug_flag = flag;
}

std::vector<Vertex> & MeshSimplifier::getVertices(){
    return vertices;
}

std::vector<unsigned int>& MeshSimplifier::getIndices(){
    return indices;
}

size_t MeshSimplifier::getVertexCount() const{
    return vertex_count;
}

size_t MeshSimplifier::getVertexSize() const{
    return vertex_size;
}

bool MeshSimplifier::saveMesh(std::vector<Vertex>& out_vertices, std::vector<unsigned int>& out_indices) const{
    if(!mesh_loaded)
        return false;
    mesh_loaded = false;    
    out_vertices = std::move(vertices);    
    out_indices = std::move(indices);
    return true;
}

bool MeshSimplifier::simplify(float target_ratio, float error_tolerance){
    // loadMesh(osg::Geometry *geometry)
    size_t target_index_count = indices.size() * target_ratio;
    if(!mesh_loaded){
        std::cerr << "Mesh is not loaded!" << std::endl;
        return false;
    }
    if(debug_flag)
        printConnectedComponents();
    remap(vertices,vertices, indices);
    if(debug_flag)
        printConnectedComponents();

    simplifyMeshWithAttributes(vertices, indices, target_index_count,1e-2);

    return true;
}

void MeshSimplifier::setLevels(float position_levels,float normal_levels,float uv_levels){
    this->position_levels = position_levels;
    this->normal_levels = normal_levels;
    this->uv_levels = uv_levels;
}

void MeshSimplifier::setSimplificationLevel(MergeLevel level){
    this->simplification_level = level;
    switch (level) {
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
}

bool MeshSimplifier::loadMesh(osg::Geometry* geometry){
    if (!extractMeshData(geometry, vertices, indices)) {
        std::cerr << "Failed to extract geometry data!" << std::endl;
        return false;
    }
    mesh_loaded = true;
    return true;
}

void MeshSimplifier::printConnectedComponents() const {
        std::unordered_map<int, std::unordered_set<int>> adjList;

    for (size_t i = 0; i < indices.size(); i += 3) {
        int v1 = indices[i];
        int v2 = indices[i + 1];
        int v3 = indices[i + 2];

        adjList[v1].insert(v2);
        adjList[v1].insert(v3);
        adjList[v2].insert(v1);
        adjList[v2].insert(v3);
        adjList[v3].insert(v1);
        adjList[v3].insert(v2);
    }

    std::unordered_set<int> visited;
    std::vector<std::unordered_set<int>> components;

    for (const auto& pair : adjList) {
        int vertex = pair.first;
        if (visited.find(vertex) == visited.end()) {
            std::unordered_set<int> component;
            searchConnectedComponents(vertex, adjList, visited, component);
            components.push_back(component);
        }
    }

    std::cout << "Number of connected components: " << components.size() << std::endl;
    // for (size_t i = 0; i < components.size(); ++i) {
    //     std::cout << "Component " << i + 1 << " has " << components[i].size() << " vertices." << std::endl;
    // }
}

void MeshSimplifier::searchConnectedComponents(int vertex, const std::unordered_map<int, std::unordered_set<int>>& adjList, 
    std::unordered_set<int>& visited, std::unordered_set<int>& component) const{
    std::stack<int> stack;
    stack.push(vertex);

    while (!stack.empty()) {
        int current = stack.top();
        stack.pop();
        
        if (visited.find(current) == visited.end()) {
            visited.insert(current);
            component.insert(current);

            // 遍历相邻的顶点
            for (int neighbor : adjList.at(current)) {
                if (visited.find(neighbor) == visited.end()) {
                    stack.push(neighbor);
                }
            }
        }
    }
}

bool MeshSimplifier::extractMeshData(osg::Geometry* geometry,
                     std::vector<Vertex>& vertices,
                     std::vector<unsigned int>& indices) {
    // 提取顶点位置
    osg::Vec3Array* vertexArray = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertexArray) {
        std::cerr << "Geometry has no vertex array!" << std::endl;
        return false;
    }

    // 提取 UV 坐标
    osg::Vec2Array* texCoordArray = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
    
    // 提取法线
    osg::Vec3Array* normalArray = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());

    // 合并位置、UV、法线到顶点缓冲区
    for (size_t i = 0; i < vertexArray->size(); ++i) {
        Vertex* vertex = new Vertex;
        const osg::Vec3& pos = (*vertexArray)[i];
        const osg::Vec2& uv = texCoordArray ? (*texCoordArray)[i] : osg::Vec2(0.0f, 0.0f); // 如果缺少UV使用默认值
        const osg::Vec3& normal = normalArray ? (*normalArray)[i] : osg::Vec3(0.0f, 0.0f, 1.0f); // 如果缺少法线使用默认值

        vertex->setPosition(pos.x(),pos.y(),pos.z());
        vertex->setNormal(normal.x(), normal.y(),normal.z());
        vertex->setUV(uv.x(), uv.y());

        vertices.push_back(*vertex);
    }

    // 每个顶点的数据大小（位置 + UV + 法线）
    vertex_size = sizeof(Vertex); // 使用 Vertex 结构体的大小
    vertex_count = vertices.size();

    // 提取索引
    for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i) {
        osg::PrimitiveSet* primitiveSet = geometry->getPrimitiveSet(i);
        if (!extractIndices(primitiveSet, indices)) {
            std::cerr << "Failed to extract indices from PrimitiveSet!" << std::endl;
            return false;
        }
    }

    return true;
}

bool MeshSimplifier::extractIndices(osg::PrimitiveSet* primitiveSet, std::vector<unsigned int>& indices){
        if (!primitiveSet) {
        std::cerr << "PrimitiveSet is null!" << std::endl;
        return false;
    }

    // 处理 DrawElementsUInt
    if (auto* drawElementsUInt = dynamic_cast<osg::DrawElementsUInt*>(primitiveSet)) {
        for (unsigned int i = 0; i < drawElementsUInt->size(); ++i) {
            indices.push_back((*drawElementsUInt)[i]);
        }
        return true;
    }

    // 处理 DrawElementsUShort
    if (auto* drawElementsUShort = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet)) {
        for (unsigned int i = 0; i < drawElementsUShort->size(); ++i) {
            indices.push_back((*drawElementsUShort)[i]);
        }
        return true;
    }

    // 处理 DrawElementsUByte
    if (auto* drawElementsUByte = dynamic_cast<osg::DrawElementsUByte*>(primitiveSet)) {
        for (unsigned int i = 0; i < drawElementsUByte->size(); ++i) {
            indices.push_back((*drawElementsUByte)[i]);
        }
        return true;
    }

    // 处理 DrawArrays
    if (auto* drawArrays = dynamic_cast<osg::DrawArrays*>(primitiveSet)) {
        // 根据起始索引和数量生成三角形索引
        unsigned int start = drawArrays->getFirst();
        unsigned int count = drawArrays->getCount();

        if (drawArrays->getMode() == GL_TRIANGLES) {
            // 直接添加三角形索引
            for (unsigned int i = 0; i < count; i += 3) {
                indices.push_back(start + i);
                indices.push_back(start + i + 1);
                indices.push_back(start + i + 2);
            }
            return true;
        } else {
            std::cerr << "Unsupported draw mode in DrawArrays: " << drawArrays->getMode() << std::endl;
            return false;
        }
    }

    // 不支持的 PrimitiveSet 类型
    std::cerr << "Unsupported PrimitiveSet type!" << std::endl;
    return false;
}

void MeshSimplifier::remap(const std::vector<Vertex>& original_vertices,
                           std::vector<Vertex>& vertices,
                           std::vector<unsigned int>& indices) {
    // 创建量化后的顶点数据
    std::vector<Vertex> quantized_vertices;
    adaptiveQuantizeAttributes(original_vertices, quantized_vertices, vertex_size,
                               position_levels, normal_levels, uv_levels);
    
    std::vector<unsigned int> remap(indices.size());

    size_t original_vertex_count = original_vertices.size();
    size_t vertex_count = meshopt_generateVertexRemap(&remap[0], &indices[0], indices.size(),
                                                      quantized_vertices.data(), original_vertex_count, vertex_size);

    // 直接创建简化后的顶点数据
    std::vector<Vertex> simplified_vertices(vertex_count);  // 栈上分配内存
    meshopt_remapVertexBuffer(reinterpret_cast<float*>(simplified_vertices.data()),
                              reinterpret_cast<const float*>(original_vertices.data()),
                              original_vertex_count, vertex_size, remap.data());

    // 创建简化后的索引数据
    std::vector<unsigned int> simplified_indices(indices.size());
    meshopt_remapIndexBuffer(simplified_indices.data(), indices.data(), indices.size(), remap.data());

    size_t indices_count = indices.size();
    meshopt_optimizeVertexCache(simplified_indices.data(), simplified_indices.data(), indices_count, vertex_count);

    // 更新原始 vertices 和 indices
    vertices = std::move(simplified_vertices);  // 直接使用 std::move 替换 vertices 的内容
    indices = std::move(simplified_indices);    // 直接使用 std::move 替换 indices 的内容
}


void simplifyMeshWithAttributes(std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    size_t vertex_size,
    size_t target_index_count,
    float target_error){

    std::vector<unsigned int> simplified_indices(indices.size());    
    // 简化时保持属性的独立性
    size_t result_count = meshopt_simplifySloppy(
        simplified_indices.data(),
        indices.data(),
        indices.size(),
        vertices.data(),
        vertices.size() / (vertex_size / sizeof(float)),
        vertex_size,
        target_index_count,
        target_error // 错误阈值
    );

    simplified_indices.resize(result_count);
    indices = std::move(simplified_indices);
}

void MeshSimplifier::simplifyMeshWithAttributes(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    size_t target_index_count,
    float target_error)
{
    const float normal_weight = 0.5f;    // 法线的权重
    const float uv_weight = 0.8f;        // UV 坐标的权重

    // 属性权重数组，位置、法线、UV
    const float attr_weights[5] = {
        normal_weight, normal_weight, normal_weight,uv_weight,uv_weight
    };

    // 用于存储简化后的索引
    std::vector<unsigned int> simplified_indices(indices.size());

    // LOD 错误
    float lod_error = 0.f;

    // 调用简化函数
    size_t result_count = meshopt_simplifyWithAttributes(
        simplified_indices.data(),       // 输出简化后的索引缓冲区
        indices.data(),                  // 输入索引缓冲区
        indices.size(),                  // 索引数量
        reinterpret_cast<float*>(vertices.data()),  // 顶点数据（位置 + UV + 法线）
        vertices.size(),                  // 顶点数量
        vertex_size,                     // 顶点大小（字节）
        reinterpret_cast<float*>(vertices.data()) + 3, // 属性缓冲区起始地址（法线，假设法线从位置偏移 3 开始）
        vertex_size,                     // 属性缓冲区步长（每顶点大小）
        attr_weights,                    // 属性权重
        5,                               // 属性维度（位置、法线、UV，共 5）
        NULL,                            // 锁定顶点的布尔数组（可选）
        target_index_count,              // 目标索引数量
        target_error,                    // 目标误差
        0,                               // 选项标志（通常为 0）
        &lod_error                       // 输出的实际误差
    );

    simplified_indices.resize(result_count);
    indices = std::move(simplified_indices);
}
