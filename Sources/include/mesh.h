

#ifndef __mesh_h
#define __mesh_h

#include "osg/Node"
#if defined (_MSC_VER) && (_MSC_VER >= 1020)

#pragma once
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4514) // unreferenced inline function has been removed
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif


//using namespace std;

#include <vector>
#include "vertex.h"
#include "triangle.h"
using namespace std;


// Mesh class.  This stores a list of vertices &
// another list of triangles (which references the vertex list)
class Mesh
{
public:
	// Constructors and Destructors
	Mesh() {_numVerts = _numTriangles = 0;};
	Mesh(char* filename); // passed name of mesh file
	~Mesh();

	Mesh(const Mesh&); // copy ctor
	Mesh& operator=(const Mesh&); // assignment op

	// Get list of vertices, triangles
	vertex& getVertex(int index) {return _vlist[index];};
	const vertex& getVertex(int index) const {return _vlist[index];};
	triangle& getTri(int index) {return _plist[index];};
	const triangle& getTri(int index) const {return _plist[index];};

	int getNumVerts() {return _numVerts;};
	void setNumVerts(int n) {_numVerts = n;};
	int getNumTriangles() {return _numTriangles;};
	void setNumTriangles(int n) {_numTriangles = n;};

	void Normalize();// center mesh around the origin & shrink to fit in [-1, 1]

	void calcOneVertNormal(unsigned vert); // recalc normal for one vertex

	void dump(); // print mesh state to cout

	bool writePlyFile(const std::string& filePath) const;

private:
	vector<vertex> _vlist; // list of vertices in mesh
	vector<triangle> _plist; // list of triangles in mesh

	int _numVerts;
	int _numTriangles;

	bool operator==(const Mesh&); // don't allow op== -- too expensive
	
	bool loadFromFile(char* filename); // load from PLY file

	void ChangeStrToLower(char* pszUpper)
	{
		for(char* pc = pszUpper; pc < pszUpper + strlen(pszUpper); pc++) {
			*pc = (char)tolower(*pc);
		}
	}

	// get bounding box for mesh
	void setMinMax(float min[3], float max[3]);

	void calcAllQMatrices(Mesh& mesh); // used for Quadrics method

	void calcVertNormals(); // Calculate the vertex normals after loading the mesh

	// Helper function for reading PLY mesh file
	bool readNumPlyVerts(FILE *&inFile, int& nVerts);
	bool readNumPlyTris(FILE *&inFile, int& nTris);
	bool readPlyHeader(FILE *&inFile);
	bool readPlyVerts(FILE *&inFile);
	bool readPlyTris(FILE *&inFile);
	bool readOsgbNode(osg::Node* node);
};

#endif // __mesh_h
