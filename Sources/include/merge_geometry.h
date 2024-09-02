#include <osg/Node>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Simplifier>
#include <string>

class MergeGeometry{
public:
    static void collectGeometries(osg::Node* node, std::vector<osg::ref_ptr<osg::Geometry>>& geometries);
    static void convertToTriangles(osg::PrimitiveSet* primitiveSet, osg::ref_ptr<osg::DrawElementsUShort> convertedIndices, unsigned int vertexOffset);
    static osg::ref_ptr<osg::DrawElementsUShort> convertPrimitiveSetToUShort(osg::PrimitiveSet* primitiveSet);
    static void applyTexturesToGeode(osg::ref_ptr<osg::Node> root, osg::ref_ptr<osg::Geode> mergedGeode);
    static bool areVerticesEqual(const osg::Vec3& v1, const osg::Vec3& v2, float tolerance = 1e-5f);
    static void removeDuplicateVertices(osg::Geometry* geometry);
    static void traverseAndRemoveDuplicates(osg::Node* node);
    
    static osg::ref_ptr<osg::Geometry> mergeGeometries(const std::vector<osg::ref_ptr<osg::Geometry>>& geometries);
    static osg::ref_ptr<osg::Geometry> mergeGeometries(osg::ref_ptr<osg::Node> root);
    static void mergeGeometries(osg::ref_ptr<osg::Node> root,std::string outputPath);
    static osg::ref_ptr<osg::Geode> mergeGeode(osg::ref_ptr<osg::Node> root);

};

namespace std {
    template <>
    struct hash<osg::Vec3> {
        std::size_t operator()(const osg::Vec3& vec) const {
            return std::hash<float>()(vec.x()) ^ (std::hash<float>()(vec.y()) << 1) ^ (std::hash<float>()(vec.z()) << 2);
        }
    };

    template <>
    struct equal_to<osg::Vec3> {
        bool operator()(const osg::Vec3& lhs, const osg::Vec3& rhs) const {
            return lhs == rhs;
        }
    };
}