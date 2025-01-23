#include "MeshSimplifier.h"
#include "osg/Geode"
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <meshoptimizer.h>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/PrimitiveSet>
#include "mesh.h"
#include "osgDB/DataTypes"
#include <osg/Material>
#include <osg/LightSource>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Group>
#include <osg/Node>
#include <osg/LightSource> // 适用于光源


osg::ref_ptr<osg::Node> loadOSGBFile(const std::string& filename) {
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
    if (!node) {
        std::cerr << "Failed to load osgb file: " << filename << std::endl;
        return nullptr;
    }
    return node;
}

void updateGeometry(osg::Geometry* geometry,
                    const std::vector<Vertex>& vertices,
                    const std::vector<unsigned int>& indices,
                    size_t vertex_size) {
    // 获取或创建 StateSet，StateSet 管理光照、材质等渲染状态
    osg::ref_ptr<osg::StateSet> stateSet = geometry->getOrCreateStateSet();
    
    // 备份原始的材质和光照设置
    osg::ref_ptr<osg::Material> originalMaterial = dynamic_cast<osg::Material*>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
    osg::ref_ptr<osg::LightSource> originalLightSource = dynamic_cast<osg::LightSource*>(stateSet->getAttribute(osg::StateAttribute::LIGHT));

    // 更新顶点位置
    osg::ref_ptr<osg::Vec3Array> newVertexArray = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> newTexCoordArray = new osg::Vec2Array();
    osg::ref_ptr<osg::Vec3Array> newNormalArray = new osg::Vec3Array();

    for (size_t i = 0; i < vertices.size(); i++) {
        const auto& vertex = vertices[i];
        // 提取位置
        newVertexArray->push_back(osg::Vec3(vertex.position[0], vertex.position[1],vertex.position[2]));

        // 提取法线
        newNormalArray->push_back(osg::Vec3(vertex.normal[0],vertex.normal[1],vertex.normal[2]));

        // 提取 UV
        newTexCoordArray->push_back(osg::Vec2(vertex.uv[0], vertex.uv[1]));
    }

    geometry->setVertexArray(newVertexArray);
    geometry->setTexCoordArray(0, newTexCoordArray);
    geometry->setNormalArray(newNormalArray);

    // 更新索引
    osg::ref_ptr<osg::DrawElementsUInt> newDrawElements = new osg::DrawElementsUInt(GL_TRIANGLES);
    for (unsigned int index : indices) {
        newDrawElements->push_back(index);
    }
    geometry->removePrimitiveSet(0, geometry->getNumPrimitiveSets());
    geometry->addPrimitiveSet(newDrawElements);

    // ---------------------------- 保留原有的材质和光照 ----------------------------
    
    // 如果原有材质存在，保留它
    if (originalMaterial) {
        stateSet->setAttributeAndModes(originalMaterial.get(), osg::StateAttribute::ON);  // 设置原有的材质
    }

    // 如果原有光源存在，保留它
    if (originalLightSource) {
        // 原始模型有光源，保留并使用
        osg::ref_ptr<osg::Group> parent = geometry->getParent(0);
        if (parent) {
            parent->addChild(originalLightSource.get());  // 将光源添加到父节点
        }
    }

    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF); 

    // 确保法线在简化后更新
    geometry->dirtyBound();  // 强制更新包围盒
}

void saveOSGBFile(const osg::ref_ptr<osg::Node>& node, const std::string& filename) {
    if (!osgDB::writeNodeFile(*node, filename)) {
        std::cerr << "Failed to save osgb file: " << filename << std::endl;
    } else {
        std::cout << "File saved: " << filename << std::endl;
    }
}

void collectGeometries(osg::Node* node, std::vector<osg::ref_ptr<osg::Geometry>>& geometries) {
    if (!node) return;

    // 检查节点是否为 Geode 类型
    osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
    if (geode) {
        // 遍历 Geode 的每个 Drawable
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
            if (geometry) {
                geometries.push_back(geometry);
            }
        }
    }

    // 如果是 Group 类型，递归遍历子节点
    osg::Group* group = dynamic_cast<osg::Group*>(node);
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            collectGeometries(group->getChild(i), geometries);
        }
    }
}

GLenum getGeometryPrimitiveTypes(osg::Geometry* geometry) {
    // 遍历 geometry 的所有 PrimitiveSet
    for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i) {
        // 获取当前的 PrimitiveSet
        osg::PrimitiveSet* primitiveSet = geometry->getPrimitiveSet(i);

        // 获取当前 PrimitiveSet 的图元类型
        auto mode = primitiveSet->getMode();
        return mode;
        // 输出图元类型
        switch (mode) {
            case osg::PrimitiveSet::POINTS:
                std::cout << "Primitive " << i << ": POINTS" << std::endl;
                break;
            case osg::PrimitiveSet::LINES:
                std::cout << "Primitive " << i << ": LINES" << std::endl;
                break;
            case osg::PrimitiveSet::LINE_STRIP:
                std::cout << "Primitive " << i << ": LINE_STRIP" << std::endl;
                break;
            case osg::PrimitiveSet::LINE_LOOP:
                std::cout << "Primitive " << i << ": LINE_LOOP" << std::endl;
                break;
            case osg::PrimitiveSet::TRIANGLES:
                std::cout << "Primitive " << i << ": TRIANGLES" << std::endl;
                break;
            case osg::PrimitiveSet::TRIANGLE_STRIP:
                std::cout << "Primitive " << i << ": TRIANGLE_STRIP" << std::endl;
                break;
            case osg::PrimitiveSet::TRIANGLE_FAN:
                std::cout << "Primitive " << i << ": TRIANGLE_FAN" << std::endl;
                break;
            case osg::PrimitiveSet::QUADS:
                std::cout << "Primitive " << i << ": QUADS" << std::endl;
                break;
            case osg::PrimitiveSet::QUAD_STRIP:
                std::cout << "Primitive " << i << ": QUAD_STRIP" << std::endl;
                break;
            case osg::PrimitiveSet::POLYGON:
                std::cout << "Primitive " << i << ": POLYGON" << std::endl;
                break;
            default:
                std::cout << "Primitive " << i << ": Unknown Primitive Type" << std::endl;
                break;
        }
    }
}

int main(int argc, char** argv) {
    std::string inputPath = R"(E:\Data\mergeData\xingzhi_building.osgb)";
    std::string outputPath = R"(E:\Data\mergeData\out\xingzhi_building_50.osgb)";
    float target_ratio = 0.5f;
    if(argc == 5){
		std::string reductionRatio = argv[1];
		std::string numIterations = argv[2];
		inputPath = argv[3];
		outputPath = argv[4];
		// std::string reductionRatio = "0.3";
		// std::string numIterations = "1";
		// inputPath = R"(E:\Data\gaunglianda\input\zhibei1.osgb)";
		// outputPath = R"(E:\Data\gaunglianda\output\zhibei1_0_3_2.osgb)";
        target_ratio = pow(std::stof(reductionRatio),std::stoi(numIterations));
	}
    else {
        std::cout<<"Usage: [reductionRatio] [numIterations] [inputPath] [outputPath]"<<std::endl;
		std::cout<<"Usage: [inputPath]"<<std::endl;
		return 1;
    }

    // 加载 osgb 文件
    osg::ref_ptr<osg::Node> node = loadOSGBFile(inputPath);
    if (!node) return 1;
    
    // std::string outputFolder = R"(E:\Data\mergeData\out)";

    // Mesh mesh;
    // mesh.extractTexturesFromNode(node, outputFolder);
    // return 0;

    // 获取 Geometry 对象
    std::vector<osg::ref_ptr<osg::Geometry>> geometries;
    collectGeometries(node.get(), geometries);

    if (geometries.empty()) {
        std::cerr << "No geometries found in the scene graph!" << std::endl;
        return 1;
    }

    for (auto& geometry : geometries) {
        if(getGeometryPrimitiveTypes(geometry)!=osg::PrimitiveSet::TRIANGLES)
            continue;
        MeshSimplifier meshSimplifier;
        meshSimplifier.setDebugFlag(false);
        meshSimplifier.loadMesh(geometry);
        meshSimplifier.setLevels(128, 1, 4);
        // meshSimplifier.setLevels(128, 1,4);
        meshSimplifier.simplify(target_ratio,1.0f);
        updateGeometry(geometry.get(), meshSimplifier.getVertices(), meshSimplifier.getIndices(), meshSimplifier.getVertexSize());
    }

    // 保存简化后的 osgb 文件
    saveOSGBFile(node, outputPath);
    return 0;
}