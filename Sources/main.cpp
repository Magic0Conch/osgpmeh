#include <cmath>
#include <iostream>
#include <string>
#include "mesh.h"
#include <osg/PolygonMode>
#include "osg/Geometry"
#include "osg/Node"
#include "osg/PrimitiveSet"
#include "osg/ref_ptr"
#include "osgUtil/Optimizer"
#include "osgUtil/Simplifier"
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include "merge_geometry.h"
// Triangle model
bool printVertices = false;

class WireframeToggleHandler : public osgGA::GUIEventHandler {
public:
    WireframeToggleHandler(osg::StateSet* stateSet)
        : _stateSet(stateSet), _wireframe(false) {
        _stateSet->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL));
    }

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) override {
        switch (ea.getEventType()) {
            case osgGA::GUIEventAdapter::KEYDOWN: {
                if (ea.getKey() == 'w') {
                    _wireframe = !_wireframe;
                    _stateSet->setAttributeAndModes(new osg::PolygonMode(
                        osg::PolygonMode::FRONT_AND_BACK,
                        _wireframe ? osg::PolygonMode::LINE : osg::PolygonMode::FILL
                    ));
                    return true;
                }
                break;
            }
            default:
                break;
        }
        return false;
    }

private:
    osg::ref_ptr<osg::StateSet> _stateSet;
    bool _wireframe;
};

void printVertexData(osg::Geometry* geometry) {
    // 获取顶点数组
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) {
        std::cerr << "Error: No vertices found!" << std::endl;
        return;
    }

    // 获取法线数组
    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
    if (!normals) {
        std::cerr << "Error: No normals found!" << std::endl;
    }

    // 获取纹理坐标数组
    osg::Vec2Array* texCoords = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
    if (!texCoords) {
        std::cerr << "Error: No texture coordinates found!" << std::endl;
    }
	std::cout << "Vertex data:" << std::endl;
	for (unsigned int i = 0; i < vertices->size(); ++i) {
        osg::Vec3 vertex = (*vertices)[i];
        std::cout << "Vertex " << i << ": (" << vertex.x() << ", " << vertex.y() << ", " << vertex.z() << ")";
        if (normals) {
            osg::Vec3 normal = (*normals)[i];
            std::cout << " Normal: (" << normal.x() << ", " << normal.y() << ", " << normal.z() << ")";
        }
        if (texCoords) {
            osg::Vec2 texCoord = (*texCoords)[i];
            std::cout << " TexCoord: (" << texCoord.x() << ", " << texCoord.y() << ")";
        }
        std::cout << std::endl;
    }
}

void traverseAndPrintPrimitiveSets(osg::Node* node) {
    if (!node) return;

    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = geode->getDrawable(i)->asGeometry();
            if (geometry) {
                std::cout << "Geometry found in Geode" << std::endl;
				printVertexData(geometry);
                for (unsigned int j = 0; j < geometry->getNumPrimitiveSets(); ++j) {
                    osg::PrimitiveSet* ps = geometry->getPrimitiveSet(j);
					
                    if (dynamic_cast<osg::DrawArrays*>(ps)) {
                        std::cout << "  PrimitiveSet " << j << " is of type DrawArrays" << std::endl;
                    } else if (dynamic_cast<osg::DrawElementsUByte*>(ps)) {
                        std::cout << "  PrimitiveSet " << j << " is of type DrawElementsUByte" << std::endl;
                    } else if (dynamic_cast<osg::DrawElementsUShort*>(ps)) {
                        std::cout << "  PrimitiveSet " << j << " is of type DrawElementsUShort" << std::endl;
                    } else if (dynamic_cast<osg::DrawElementsUInt*>(ps)) {
                        std::cout << "  PrimitiveSet " << j << " is of type DrawElementsUInt" << std::endl;
                    } else {
                        std::cout << "  PrimitiveSet " << j << " is of unknown type" << std::endl;
                    }
                }
		    }
        }
    }

    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            traverseAndPrintPrimitiveSets(group->getChild(i));
        }
    }
}
osg::ref_ptr<osg::Node> mergeGeometry(std::string inputPath){
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(inputPath);
    if(printVertices)
        traverseAndPrintPrimitiveSets(loadedModel);
    if (!loadedModel) {
        std::cerr << "Error: Failed to load osgb file " << inputPath << std::endl;
    }
    std::string outputPath = R"(E:\work\2409\Data\NNU-MiniCIM\out\loaded.osgb)";
    auto mergedGeode = MergeGeometry::mergeGeode(loadedModel);
    std::cout<<"Sucess!"<<std::endl;
    return mergedGeode;
}

void simplyfyMesh(float reductionRatio, int numIterations, std::string inputPath, std::string outputPath){
	std::cout<<"Input file: "<<inputPath<<std::endl;
    // 启动前先合并顶点
	// osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(inputPath);
    auto node = mergeGeometry(inputPath);
    if(node == nullptr){
        std::cerr << "Error: Failed to load osgb file " << inputPath << std::endl;
        return;
    }
	std::cout<<"Read file success!"<<std::endl;

	if(printVertices){
		traverseAndPrintPrimitiveSets(node);
	}
	else {
		std::cout<<"Print vertices disabled!"<<std::endl;	
	}
	std::cout<<"Reduction ratio: "<<reductionRatio<<std::endl;
	std::cout<<"Num iterations: "<<numIterations<<std::endl;
	osgUtil::Optimizer optimizer;
	optimizer.optimize(node.get());
	if (!node) {
        std::cerr << "Error: unable to load input file " << inputPath << std::endl;
        return;
    }
	osgUtil::Simplifier simple;
	simple.setSmoothing( 0 );	
	reductionRatio =pow(reductionRatio,numIterations);
	std::cout<<"SimplyMesh Start!"<<std::endl;
	simple.setSampleRatio( reductionRatio );
	node->accept( simple );
    std::cout<<"Simplify mesh success!Start to write File!"<<std::endl;
	if (!osgDB::writeNodeFile(*node, outputPath)) {
        std::cerr << "Error: unable to write output file " << outputPath << std::endl;
        return;
    }
    std::cout<<"Output file: "<<outputPath<<std::endl;
	return;
	osg::ref_ptr<osg::Geode> geode = node->clone(osg::CopyOp::DEEP_COPY_ALL)->asNode()->asGeode();	
	if (!node) {
        std::cerr << "Error: unable to load input file " << inputPath << std::endl;
        return;
    }	
	Mesh mesh;
	auto geom  = mesh.readOsgbNode(node, reductionRatio,numIterations);
	geode->removeDrawables(0, geode->getNumDrawables());
    geode->addDrawable(geom);
    // return osgDB::writeNodeFile(*geode, "output.osgb");
    if (!osgDB::writeNodeFile(*geode, outputPath)) {
        std::cerr << "Error: unable to write output file " << outputPath << std::endl;
        return;
    }
	std::cout<<"Simplify mesh success!"<<std::endl;
	std::cout<<"Output file: "<<outputPath<<std::endl;
}	



int main(int argc, char** argv){
    // std::string reductionRatio = "0.3";
    // std::string numIterations = "1";
    // std::string inputPath = R"(E:\Data\gaunglianda\input\zhibei1.osgb)";
    // std::string outputPath = R"(E:\Data\gaunglianda\output\zhibei1_0_3_2.osgb)";
    // simplyfyMesh(std::stof(reductionRatio), std::stoi(numIterations), inputPath, outputPath);
    // return 0;

	for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "-p") {
            printVertices = true;
            break;
        }
    }
	if(argc == 5 || (printVertices && argc == 6)){
		std::string reductionRatioStr = argv[1];
		std::string numIterationsStr = argv[2];
		std::string inputPath = argv[3];
		std::string outputPath = argv[4];
        auto reductionRatio = std::stof(reductionRatioStr);
        auto numIterations = std::stoi(numIterationsStr);
		
		// float reductionRatio = 0.3;
		// int numIterations = 1;
		// std::string inputPath = R"(E:\Data\osgb\zhibei1.osgb)";
		// std::string outputPath = R"(E:\Data\gaunglianda\output\zhibei1_0_3_2.osgb)";
		simplyfyMesh(reductionRatio, numIterations, inputPath, outputPath);
		return 0;
	}
	else if(argc == 2 || (printVertices && argc == 3)){
		std::string inputPath = argv[1];
		
		osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(inputPath);
		if(printVertices)
			traverseAndPrintPrimitiveSets(loadedModel);
		if (!loadedModel) {
		    std::cerr << "Error: Failed to load osgb file " << inputPath << std::endl;
		    return 1;
		}

		osgViewer::Viewer viewer;
		osg::ref_ptr<osg::StateSet> stateSet = loadedModel->getOrCreateStateSet();
		viewer.addEventHandler(new WireframeToggleHandler(stateSet));
		viewer.addEventHandler(new osgViewer::StatsHandler);
		viewer.setSceneData(loadedModel);
		int windowX = 100; // 窗口左上角的 X 坐标
		int windowY = 100; // 窗口左上角的 Y 坐标
		int windowWidth = 800; // 窗口宽度
		int windowHeight = 600; // 窗口高度
		viewer.setUpViewInWindow(windowX, windowY, windowWidth, windowHeight);
		return viewer.run();
	}
    // else if(argc == 1){
    //     std::string inputPath = R"(E:\work\2409\Data\NNU-MiniCIM\osgb\canteen.osgb)";
    //     mergeGeometry(inputPath);
    // }
	else {
		std::cout<<"Usage: [reductionRatio] [numIterations] [inputPath] [outputPath]"<<std::endl;
		std::cout<<"Usage: [inputPath]"<<std::endl;
		return 1;
	}
    // string osgbFile = "E:\\work\\Data\\out\\zhibei1.osgb";

}