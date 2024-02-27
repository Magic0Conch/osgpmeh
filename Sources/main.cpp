#include <iostream>
#include <string>
#include <windows.h>		// Header File For Windows
#include <sys/stat.h>
#include <cstdio>
#include <fstream>
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4514) // unreferenced inline function has been removed
#pragma warning(disable:4786) // disable "identifier was truncated to '255' characters in the browser information" warning in Visual C++ 6*
#endif
#include "pmesh.h"
#include "mesh.h"


#include "osgDB/ReadFile"
#include <osgViewer/Viewer>
#include <json11.hpp>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/StateSet>
#include <osg/LineWidth>
#include <osg/PolygonMode>
// Triangle model
Mesh* g_pMesh = NULL;

// Progressive Mesh
PMesh* g_pProgMesh = NULL;

// Edge Collapse Options
PMesh::EdgeCost g_edgemethod = PMesh::QUADRICTRI;
// file name
char g_filename[256] = {'\0'};

void loadJson(){
	static char szFilter[] = "Json files (*.json)\0*.json\0";
	OPENFILENAME ofn;
	char pszFileLocn[256] = { '\0' };

	// Set up OPENFILENAME struct to use commond dialog box for open

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(ofn);	// size of struct
	ofn.hwndOwner = NULL;			// window that owns Dlg
	ofn.lpstrFilter = szFilter;		// Filter text
	ofn.lpstrFile = pszFileLocn;		// File name string
	ofn.nMaxFile = sizeof(pszFileLocn); // size of file name
	ofn.Flags = OFN_HIDEREADONLY;	// don't display "Read Only"
	ofn.lpstrDefExt = "Json";		// extension name
	ofn.lpstrTitle = "Open Input Config File"; // title of dlg box

	// call common dlg control for file open
	if (!GetOpenFileName(&ofn)) {
		return;
	}

	// see if file exists
	struct stat fileStat;
	if (stat(ofn.lpstrFile, &fileStat))
	{
		char errormsg[1024];
		sprintf(errormsg, "%s not found.", ofn.lpstrFile);
		MessageBox(NULL, errormsg, "File Not Found Error", MB_OK | MB_ICONINFORMATION);
		return;
	}

	delete g_pMesh;
	g_pMesh = NULL; // not necessary, but a nice CYA habit
	delete g_pProgMesh;
	g_pProgMesh = NULL;
	
	string jsonFilePath = ofn.lpstrFile;

    std::ifstream assetJsonFile(jsonFilePath);
    if(!assetJsonFile){
        std::cout<<"Open file: " + jsonFilePath + " failed!";
        assert(0);
    }
    std::stringstream buffer;
    buffer << assetJsonFile.rdbuf();
    std::string assetJsonText(buffer.str());
    
    std::string errorMessage;
    auto &&assetJson = json11::Json::parse(assetJsonText,errorMessage);
    
	std::string inputPath = assetJson["input_path"].string_value();
	std::string outputPath = assetJson["output_path"].string_value();
	int numIterations = assetJson["num_iterations"].int_value();
	float reductionRatio = assetJson["reduction_ratio"].number_value();
	
	char* inputPathCStr = new char[inputPath.length() + 1];
	std::strcpy(inputPathCStr, inputPath.c_str());

	g_pMesh = new Mesh(inputPathCStr);
	std::strcpy(g_filename, inputPathCStr);
	// g_pMesh-> writePlyFile(outputPath);
	if (g_pMesh) g_pMesh->Normalize();// center mesh around the origin & shrink to fit

	g_pProgMesh = new PMesh(g_pMesh, g_edgemethod);

	// reset the position of the mesh
	//g_pWindow->resetOrientation();

	//g_pWindow->displayWindowTitle();
	delete[] inputPathCStr;

	for (int i = 0; i < numIterations; i++) {
		if (g_pProgMesh)
		{
			const int REDUCE_TRI_PERCENT = reductionRatio * 100;	// when page up/page down, inc/dec # tris by this percent 
			const int NUM_PAGEUPDN_INTERVALS = 100 / REDUCE_TRI_PERCENT;
			int size = (g_pProgMesh->numEdgeCollapses()) / NUM_PAGEUPDN_INTERVALS;
			if (size == 0) size = 1;
			bool ret = true;
			for (int i = 0; ret && i < size; ++i) {
				ret = g_pProgMesh->collapseEdge();
			}
			if (!ret) MessageBeep(0);
		}
	}
	g_pProgMesh->getNewMesh().writeFile(outputPath);
}

int main(int argc, char** argv){
    // loadJson();
	// return 0;
    string osgbFile = "E:\\work\\C++\\jmspmesh\\out\\shitang.osgb";
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(osgbFile);

    if (!loadedModel) {
        std::cerr << "Error: Failed to load osgb file " << osgbFile << std::endl;
        return 1;
    }

    osgViewer::Viewer viewer;
	osg::ref_ptr<osg::StateSet> stateSet = loadedModel->getOrCreateStateSet();

	osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
    lineWidth->setWidth(2.0f); // 设置线宽
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);

	stateSet->setMode(GL_POLYGON_MODE, osg::StateAttribute::ON);
    stateSet->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));

	// osg::ref_ptr<osg::StateSet> stateSet = loadedModel;
    // osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
    // polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE); // 设置线框模式
    // stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

    // loadedModel->setStateSet(stateSet);
    viewer.setSceneData(loadedModel);

    return viewer.run();
}