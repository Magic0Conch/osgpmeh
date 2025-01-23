#include "osg/Geometry"
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Texture2D>
using namespace std;

class Mesh
{
public:
    osg::Geometry* readOsgbNode(osg::Node* node,float ratio,int numIterations);
    void combineGeometries(osg::Node* node, osg::Geometry* combinedGeometry,osg::ref_ptr<osg::StateSet> combinedStateSet);
    void extractTexturesFromNode(osg::Node* node, const std::string& output_folder);
    void collectTextures(osg::Node* node, std::vector<osg::ref_ptr<osg::Texture2D>>& textures);
private:
    unsigned int _numVerts = 0;
};

