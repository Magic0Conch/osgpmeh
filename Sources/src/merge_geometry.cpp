#include "merge_geometry.h"
#include "osg/PrimitiveSet"
#include <osg/Texture2D>
#include <filesystem>
#include <unordered_map>



void MergeGeometry::applyTexturesToGeode(osg::ref_ptr<osg::Node> root, osg::ref_ptr<osg::Geode> mergedGeode) {
    // 自定义的NodeVisitor，用于遍历所有的Geode节点并收集纹理
    class TextureCollector : public osg::NodeVisitor {
    public:
        TextureCollector() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}

        void apply(osg::Geode& geode) override {
            for (unsigned int i = 0; i < geode.getNumDrawables(); ++i) {
                osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
                if (geometry) {
                    osg::StateSet* stateSet = geometry->getStateSet();
                    if (stateSet) {
                        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
                        if (texture) {
                            // 检查纹理是否已经被添加
                            if (uniqueTextures.find(texture) == uniqueTextures.end()) {
                                uniqueTextures.insert(texture); // 插入新纹理
                                textures.push_back(texture);    // 添加到纹理列表中
                            }
                        }
                    }
                }
            }
        }

        void saveAndSetTextures(const std::string& outputDir) {
            // Ensure the output directory exists
            std::filesystem::create_directories(outputDir + "/images");

            int textureIndex = 0;
            for (auto& texture : textures) {
                if (texture) {
                    // Get the texture image
                    osg::ref_ptr<osg::Image> image = texture->getImage();
                    if (!image) {
                        std::cerr << "Error: Texture has no associated image!" << std::endl;
                        continue;
                    }

                    // Generate a filename for the texture
                    std::ostringstream filename;
                    filename << outputDir << "/images/texture_" << std::setw(3) << std::setfill('0') << textureIndex << ".jpg";

                    // Save the texture image to file
                    if (!osgDB::writeImageFile(*image, filename.str())) {
                        std::cerr << "Failed to save texture to file: " << filename.str() << std::endl;
                        continue;
                    }

                    // // Create a new texture with the saved image
                    // osg::ref_ptr<osg::Texture2D> newTexture = new osg::Texture2D;
                    // newTexture->setImage(osgDB::readImageFile(filename.str()));

                    // // Update StateSet with the new texture
                    // mergedGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, newTexture, osg::StateAttribute::ON);

                    ++textureIndex;
                }
            }
        }

        std::vector<osg::ref_ptr<osg::Texture2D>> textures;
        std::set<osg::ref_ptr<osg::Texture2D>> uniqueTextures; // 用于去重的集合
    };

    TextureCollector collector;
    root->accept(collector);
    std::string outputPath = R"(E:\work\2409\Data\NNU-MiniCIM\out)";
    collector.saveAndSetTextures(outputPath);

    // for (auto& texture : collector.textures) {
    //     if (texture) {
    //         mergedGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    //     }
    // }
}

osg::ref_ptr<osg::DrawElementsUShort> MergeGeometry::convertPrimitiveSetToUShort(osg::PrimitiveSet* primitiveSet) {
    osg::ref_ptr<osg::DrawElementsUShort> convertedIndices = new osg::DrawElementsUShort(primitiveSet->getMode());

    if (auto drawElementsUShort = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet)) {
        // 如果已经是 DrawElementsUShort 类型，直接拷贝数据
        convertedIndices->assign(drawElementsUShort->begin(), drawElementsUShort->end());
    } else if (auto drawElementsUByte = dynamic_cast<osg::DrawElementsUByte*>(primitiveSet)) {
        // 将 DrawElementsUByte 转换为 DrawElementsUShort
        for (auto index : *drawElementsUByte) {
            convertedIndices->push_back(static_cast<unsigned short>(index));
        }
    } else if (auto drawElementsUInt = dynamic_cast<osg::DrawElementsUInt*>(primitiveSet)) {
        // 将 DrawElementsUInt 转换为 DrawElementsUShort
        for (auto index : *drawElementsUInt) {
            convertedIndices->push_back(static_cast<unsigned short>(index));
        }
    } else {
        // 对于其他类型的 PrimitiveSet，可以添加更多处理逻辑
        osg::notify(osg::WARN) << "Unsupported primitive set type for conversion to DrawElementsUShort." << std::endl;
    }

    return convertedIndices;
}

void MergeGeometry::convertToTriangles(osg::PrimitiveSet* primitiveSet, osg::ref_ptr<osg::DrawElementsUShort> convertedIndices, unsigned int vertexOffset) {
    if (primitiveSet->getMode() == GL_TRIANGLE_FAN) {
        // Convert GL_TRIANGLE_FAN to GL_TRIANGLES
        auto drawElements = convertPrimitiveSetToUShort(primitiveSet);
        if (drawElements) {
            for (unsigned int i = 1; i < drawElements->size() - 1; ++i) {
                convertedIndices->push_back((*drawElements)[0] + vertexOffset);
                convertedIndices->push_back((*drawElements)[i] + vertexOffset);
                convertedIndices->push_back((*drawElements)[i + 1] + vertexOffset);
            }
        }
    } else if (primitiveSet->getMode() == GL_TRIANGLE_STRIP) {
        // Convert GL_TRIANGLE_STRIP to GL_TRIANGLES
        auto drawElements = convertPrimitiveSetToUShort(primitiveSet);
        if (drawElements) {
            for (unsigned int i = 0; i < drawElements->size() - 2; ++i) {
                if (i % 2 == 0) {
                    convertedIndices->push_back((*drawElements)[i] + vertexOffset);
                    convertedIndices->push_back((*drawElements)[i + 1] + vertexOffset);
                    convertedIndices->push_back((*drawElements)[i + 2] + vertexOffset);
                } else {
                    convertedIndices->push_back((*drawElements)[i] + vertexOffset);
                    convertedIndices->push_back((*drawElements)[i + 2] + vertexOffset);
                    convertedIndices->push_back((*drawElements)[i + 1] + vertexOffset);
                }
            }
        }
    }
}

osg::ref_ptr<osg::Geometry> MergeGeometry::mergeGeometries(const std::vector<osg::ref_ptr<osg::Geometry>>& geometries) {
    osg::ref_ptr<osg::Geometry> mergedGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> mergedVertices = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUInt> mergedIndices = new osg::DrawElementsUInt(GL_TRIANGLES);
    osg::ref_ptr<osg::Vec2Array> mergedTexCoords = new osg::Vec2Array;  // For texture coordinates
    osg::ref_ptr<osg::Vec3Array> mergedNormals = new osg::Vec3Array;    // For normals

    unsigned int vertexOffset = 0;

    for (const auto& geometry : geometries) {
        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
        if (!vertices) continue;

        // Merge vertices
        for (const auto& vertex : *vertices) {
            mergedVertices->push_back(vertex);
        }

        // Merge indices
        for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i) {
            osg::PrimitiveSet* primitiveSet = geometry->getPrimitiveSet(i);
            // osg::DrawElementsUShort* drawElements = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet);
            // if (drawElements) {
            //     for (const auto& index : *drawElements) {
            //         mergedIndices->push_back(index + vertexOffset);
            //     }
            // }
             if (primitiveSet->getMode() == GL_TRIANGLES) {
                if (auto drawElementsUShort = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet)) {
                    for (const auto& index : *drawElementsUShort) {
                        mergedIndices->push_back(index + vertexOffset);
                    }
                } else if (auto drawElementsUInt = dynamic_cast<osg::DrawElementsUInt*>(primitiveSet)) {
                    for (const auto& index : *drawElementsUInt) {
                        mergedIndices->push_back(static_cast<unsigned short>(index) + vertexOffset);
                    }
                }
            } else if (primitiveSet->getMode() == GL_TRIANGLE_FAN || primitiveSet->getMode() == GL_TRIANGLE_STRIP) {
                osg::ref_ptr<osg::DrawElementsUShort> convertedIndices = new osg::DrawElementsUShort(GL_TRIANGLES);
                // Convert to GL_TRIANGLES
                convertToTriangles(primitiveSet, convertedIndices, vertexOffset);
                for (const auto& index : *convertedIndices) {
                    mergedIndices->push_back(index);
                }
            } else {
                // Handle other primitive types if needed
                // Example: GL_QUADS, GL_POLYGON, etc.
            }
        }

        osg::Vec2Array* texCoords = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());

        // Merge texture coordinates
        if (texCoords) {
            for (const auto& texCoord : *texCoords) {
                mergedTexCoords->push_back(texCoord);
            }
        }

        // Merge normals
        if (normals) {
            for (const auto& normal : *normals) {
                mergedNormals->push_back(normal);
            }
        }

        vertexOffset += vertices->size();
    }

    mergedGeometry->setVertexArray(mergedVertices);
    mergedGeometry->addPrimitiveSet(mergedIndices);
    if (!mergedTexCoords->empty()) {
        mergedGeometry->setTexCoordArray(0, mergedTexCoords);
    }
    if (!mergedNormals->empty()) {
        mergedGeometry->setNormalArray(mergedNormals);
        mergedGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    return mergedGeometry;
}

// Function to traverse the scene graph and collect all geometries
void MergeGeometry::collectGeometries(osg::Node* node, std::vector<osg::ref_ptr<osg::Geometry>>& geometries) {
    if (!node) return;

    osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
            if (geometry) {
                geometries.push_back(geometry);
            }
        }
    }

    osg::Group* group = dynamic_cast<osg::Group*>(node);
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            collectGeometries(group->getChild(i), geometries);
        }
    }
}

osg::ref_ptr<osg::Geometry> MergeGeometry::mergeGeometries(osg::ref_ptr<osg::Node> root){
    std::vector<osg::ref_ptr<osg::Geometry>> geometries;
    collectGeometries(root, geometries);
    auto mergedGeometry = mergeGeometries(geometries);
    
    return mergedGeometry;
    
}

void MergeGeometry::mergeGeometries(osg::ref_ptr<osg::Node> root,std::string outputPath){
    auto mergedGeometry = mergeGeometries(root);
    auto mergedGeode = new osg::Geode;
    mergedGeode->addDrawable(mergedGeometry);
    // applyTexturesToGeode(root, mergedGeode);
    if (root->getStateSet()) {
        osg::ref_ptr<osg::StateSet> stateSetCopy = new osg::StateSet(*root->getStateSet(), osg::CopyOp::SHALLOW_COPY);
        mergedGeode->setStateSet(stateSetCopy);
    }
    traverseAndRemoveDuplicates(mergedGeode);
    if (!osgDB::writeNodeFile(*mergedGeode, outputPath)) {
        std::cerr << "Failed to write file: " << outputPath << std::endl;
    }
}

bool MergeGeometry::areVerticesEqual(const osg::Vec3& v1, const osg::Vec3& v2, float tolerance) {
    return (fabs(v1.x() - v2.x()) < tolerance &&
            fabs(v1.y() - v2.y()) < tolerance &&
            fabs(v1.z() - v2.z()) < tolerance);
}

// Function to remove duplicate vertices from a geometry
void MergeGeometry::removeDuplicateVertices(osg::Geometry* geometry) {
    if (!geometry) return;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return;

    std::unordered_map<osg::Vec3, unsigned int> uniqueVertices;
    std::vector<unsigned int> indexMap(vertices->size());

    osg::ref_ptr<osg::Vec3Array> newVertices = new osg::Vec3Array;
    for (unsigned int i = 0; i < vertices->size(); ++i) {
        const osg::Vec3& vertex = (*vertices)[i];
        bool found = false;

        for (const auto& [uniqueVertex, index] : uniqueVertices) {
            if (areVerticesEqual(vertex, uniqueVertex)) {
                indexMap[i] = index;
                found = true;
                break;
            }
        }

        if (!found) {
            unsigned int newIndex = newVertices->size();
            uniqueVertices[vertex] = newIndex;
            newVertices->push_back(vertex);
            indexMap[i] = newIndex;
        }
    }

    geometry->setVertexArray(newVertices);

    for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i) {
        osg::PrimitiveSet* primitiveSet = geometry->getPrimitiveSet(i);
        osg::DrawElementsUInt* drawElementsUInt = dynamic_cast<osg::DrawElementsUInt*>(primitiveSet);
        osg::DrawElementsUShort* drawElementsUShort = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet);
        osg::DrawElementsUByte* drawElementsUByte = dynamic_cast<osg::DrawElementsUByte*>(primitiveSet);

        if (drawElementsUInt) {
            for (unsigned int j = 0; j < drawElementsUInt->size(); ++j) {
                (*drawElementsUInt)[j] = indexMap[(*drawElementsUInt)[j]];
            }
        } else if (drawElementsUShort) {
            for (unsigned int j = 0; j < drawElementsUShort->size(); ++j) {
                (*drawElementsUShort)[j] = indexMap[(*drawElementsUShort)[j]];
            }
        } else if (drawElementsUByte) {
            for (unsigned int j = 0; j < drawElementsUByte->size(); ++j) {
                (*drawElementsUByte)[j] = indexMap[(*drawElementsUByte)[j]];
            }
        }
    }
}

// Function to traverse the scene graph and remove duplicate vertices
void MergeGeometry::traverseAndRemoveDuplicates(osg::Node* node) {
    osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
            if (geometry) {
                removeDuplicateVertices(geometry);
            }
        }
    }

    osg::Group* group = dynamic_cast<osg::Group*>(node);
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            traverseAndRemoveDuplicates(group->getChild(i));
        }
    }
}

osg::ref_ptr<osg::Geode> MergeGeometry::mergeGeode(osg::ref_ptr<osg::Node> root){
    auto mergedGeometry = mergeGeometries(root);
    auto mergedGeode = new osg::Geode;
    mergedGeode->addDrawable(mergedGeometry);
    // applyTexturesToGeode(root, mergedGeode);
    if (root->getStateSet()) {
        osg::ref_ptr<osg::StateSet> stateSetCopy = new osg::StateSet(*root->getStateSet(), osg::CopyOp::SHALLOW_COPY);
        mergedGeode->setStateSet(stateSetCopy);
    }
    traverseAndRemoveDuplicates(mergedGeode);
    return mergedGeode;
}