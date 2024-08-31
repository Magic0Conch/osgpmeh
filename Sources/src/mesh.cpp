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