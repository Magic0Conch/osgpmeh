#include "osg/Geometry"
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
using namespace std;

class Mesh
{
public:
    osg::Geometry* readOsgbNode(osg::Node* node,float ratio,int numIterations);
    void combineGeometries(osg::Node* node, osg::Geometry* combinedGeometry,osg::ref_ptr<osg::StateSet> combinedStateSet);

private:
    unsigned int _numVerts = 0;
};

