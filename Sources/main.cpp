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
	g_pProgMesh->getNewMesh().writePlyFile(outputPath);
}

int main(int argc, char** argv){
    loadJson();
	return 0;
    cout<<"Hello"<<endl;
    string osgbFile = "E:\\work\\Data\\NNU_CIM_DataSet_2024.01.05\\CIM1DataSet\\SyntheticModel\\ExternalData\\terrain+region\\30DEM\\30DEM.osgb";
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(osgbFile);

    if (!loadedModel) {
        std::cerr << "Error: Failed to load osgb file " << osgbFile << std::endl;
        return 1;
    }

    osgViewer::Viewer viewer;

    viewer.setSceneData(loadedModel);

    return viewer.run();
}