#include "mesh.h"
#include "osg/Node"
#include "osg/ref_ptr"
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Simplifier>
#include <iostream>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Simplifier>
#include <osg/StateSet>
#include <osg/Material>
#include <iostream>
#include <osg/Texture2D>
#include <unordered_set>
void Mesh::combineGeometries(osg::Node* node, osg::Geometry* combinedGeometry, osg::ref_ptr<osg::StateSet> combinedStateSet) {
    if (!node) return;

    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = geode->getDrawable(i)->asGeometry();
            if (geometry) {
                osg::Vec3Array* verticesArray = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                osg::Vec3Array* combinedVertices = dynamic_cast<osg::Vec3Array*>(combinedGeometry->getVertexArray());
                if (!combinedVertices) {
                    combinedVertices = new osg::Vec3Array;
                    combinedGeometry->setVertexArray(combinedVertices);
                }

                unsigned int vertexOffset = combinedVertices->size();
                if (verticesArray) {
                    for (unsigned int j = 0; j < verticesArray->size(); ++j) {
                        combinedVertices->push_back(verticesArray->at(j));
                    }
                }

                osg::Vec3Array* normalsArray = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
                osg::Vec3Array* combinedNormals = dynamic_cast<osg::Vec3Array*>(combinedGeometry->getNormalArray());
                if (!combinedNormals) {
                    combinedNormals = new osg::Vec3Array;
                    combinedGeometry->setNormalArray(combinedNormals, osg::Array::BIND_PER_VERTEX);
                }

                if (normalsArray) {
                    for (unsigned int j = 0; j < normalsArray->size(); ++j) {
                        combinedNormals->push_back(normalsArray->at(j));
                    }
                }

                osg::Vec2Array* texCoordsArray = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
                osg::Vec2Array* combinedTexCoords = dynamic_cast<osg::Vec2Array*>(combinedGeometry->getTexCoordArray(0));
                if (!combinedTexCoords) {
                    combinedTexCoords = new osg::Vec2Array;
                    combinedGeometry->setTexCoordArray(0, combinedTexCoords);
                }

                if (texCoordsArray) {
                    for (unsigned int j = 0; j < texCoordsArray->size(); ++j) {
                        combinedTexCoords->push_back(texCoordsArray->at(j));
                    }
                }

                osg::Vec4Array* colorArray = dynamic_cast<osg::Vec4Array*>(geometry->getColorArray());
                osg::Vec4Array* combinedColors = dynamic_cast<osg::Vec4Array*>(combinedGeometry->getColorArray());
                if (!combinedColors) {
                    combinedColors = new osg::Vec4Array;
                    combinedGeometry->setColorArray(combinedColors, osg::Array::BIND_PER_VERTEX);
                }

                if (colorArray) {
                    for (unsigned int j = 0; j < colorArray->size(); ++j) {
                        combinedColors->push_back(colorArray->at(j));
                    }
                }
                // for (int ipr = 0; ipr < numP; ++ipr) {
                //     combinedGeometry->addPrimitiveSet(prset);
                // }
                for (unsigned int ipr = 0; ipr < geometry->getNumPrimitiveSets(); ++ipr) {
                    osg::PrimitiveSet* prset = geometry->getPrimitiveSet(ipr);
                    // osg::PrimitiveSet* prset = geometry->getPrimitiveSet(ipr)->clone(osg::CopyOp::DEEP_COPY_ALL);
                    osg::DrawElementsUInt* drawElements = dynamic_cast<osg::DrawElementsUInt*>(prset);
                    if (drawElements) {
                        for (unsigned int k = 0; k < drawElements->size(); ++k) {
                            (*drawElements)[k] += vertexOffset;
                        }
                    }
                    combinedGeometry->addPrimitiveSet(prset);
                }

                osg::StateSet* stateSet = geometry->getStateSet();
                if (stateSet) {
                    combinedStateSet->merge(*stateSet);
                }
            }
        }
    }

    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            combineGeometries(group->getChild(i), combinedGeometry, combinedStateSet);
        }
    }
}

osg::Geometry* Mesh::readOsgbNode(osg::Node* node, float ratio, int numIterations) {
    osg::Geometry* combinedGeometry = new osg::Geometry;
    osg::ref_ptr<osg::StateSet> combinedStateSet = new osg::StateSet;
    combineGeometries(node, combinedGeometry, combinedStateSet);

    osgUtil::Simplifier simplifier;
	simplifier.setSmoothing(false);
    simplifier.setSampleRatio(ratio);
    for (int i = 0; i < numIterations; i++) {
        std::cout << "Simplify iteration: " << i << std::endl;
        std::cout << "Num vertices: " << combinedGeometry->getVertexArray()->getTotalDataSize() << std::endl;
        std::cout << "Ratio: " << simplifier.getSampleRatio() << std::endl;
        combinedGeometry->accept(simplifier);
    }
    combinedGeometry->setStateSet(combinedStateSet);
    return combinedGeometry;
}

void Mesh::extractTexturesFromNode(osg::Node* node, const std::string& output_folder) {
    if (!node) {
        std::cerr << "Input node is null!" << std::endl;
        return;
    }

    // 存储唯一纹理
    std::unordered_set<std::string> saved_texture_files;
    std::vector<osg::ref_ptr<osg::Texture2D>> textures;

    // 遍历节点，收集纹理
    collectTextures(node, textures);

    // 保存纹理（避免重复）
    for (size_t i = 0; i < textures.size(); ++i) {
        osg::ref_ptr<osg::Texture2D> texture = textures[i];
        if (texture && texture->getImage()) {
            std::string texture_filename = output_folder + "/texture_" + std::to_string(i) + ".jpg";
            
            // 检查纹理是否已经保存
            if (saved_texture_files.find(texture_filename) == saved_texture_files.end()) {
                osgDB::writeImageFile(*texture->getImage(), texture_filename);
                saved_texture_files.insert(texture_filename); // 标记该纹理已保存
                std::cout << "Texture saved to: " << texture_filename << std::endl;
            }
        }
    }
}

void Mesh::collectTextures(osg::Node* node, std::vector<osg::ref_ptr<osg::Texture2D>>& textures) {
    if (!node) return;

    // 如果节点是 Geode 类型，检查它的 Drawable（几何体）
    osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Drawable* drawable = geode->getDrawable(i);
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
            if (geometry) {
                // 获取几何体的 StateSet，检查其中的纹理
                osg::StateSet* stateSet = geometry->getStateSet();
                if (stateSet) {
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
                    if (texture && texture->getImage()) {
                        textures.push_back(texture);
                    }
                }
            }
        }
    }

    // 如果是 Group 类型，递归遍历子节点
    osg::Group* group = dynamic_cast<osg::Group*>(node);
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            collectTextures(group->getChild(i), textures);
        }
    }
}