
#include "mesh.h"
#include "osg/Vec2"
#include <fstream>
#include <string>

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4514) // unreferenced inline function has been removed
#endif

#include <assert.h>
#include <float.h>
#include <iostream>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>
#include "osgDB/ReadFile"
#include <osgDB/WriteFile>
#include <osg/Group>


const char* getFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename && *(dot + 1)) { 
        return dot + 1;
    }
    return "";
}

Mesh::Mesh(char* filename)
{
	_numVerts = _numTriangles = 0;
	if (!loadFromFile(filename))
	{
		// we failed to load mesh from the file
		_numVerts = _numTriangles = 0;
		_vlist.clear();
		_plist.clear();
	}
}

Mesh::Mesh(const Mesh& m)
{
	_numVerts = m._numVerts;
	_numTriangles = m._numTriangles;
	_vlist = m._vlist; // NOTE: triangles are still pointing to original mesh
	_plist = m._plist;
	// NOTE: should reset tris in _vlist, _plist
}

Mesh& Mesh::operator=(const Mesh& m)
{
	if (this == &m) return *this; // don't assign to self
	_numVerts = m._numVerts;
	_numTriangles = m._numTriangles;
	_vlist = m._vlist; // NOTE: triangles are still pointing to original mesh
	_plist = m._plist;
	// NOTE: should reset tris in _vlist, _plist
	return *this;
}

Mesh::~Mesh()
{
	_numVerts = _numTriangles = 0;
	_vlist.erase(_vlist.begin(), _vlist.end());
	_plist.erase(_plist.begin(), _plist.end());
}

// Helper function for reading PLY mesh file�����붥����
bool Mesh::readNumPlyVerts(FILE *&inFile, int& nVerts)
{
	// Read # of verts
	bool bElementFound = false;
	/* Get number of vertices in mesh*/
	for(;;)
	{       
		char tempStr[1024];
		fscanf(inFile, "%s", tempStr);
		if (feof(inFile))
		{
			MessageBox(NULL, "Reached End of File and string \"element vertex\" not found!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}

		/* change tempStr to lower case */
		ChangeStrToLower(tempStr);

		if (bElementFound && !strncmp(tempStr, "vertex", 6))
		{
			break;
		}

		if (!strncmp(tempStr, "element", 7))
		{
			bElementFound = true;
			continue;
		}
	}

	fscanf(inFile, "%d", &nVerts); 
	if (feof(inFile))
	{
		MessageBox(NULL, "Reached End of File before \"element face\" found!\n",
			NULL, MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

// Helper function for reading PLY mesh file�����������θ���
bool Mesh::readNumPlyTris(FILE *&inFile, int& nTris)
{
	bool bElementFound = false;
	/* Get number of faces in mesh*/
	for(;;)
	{
		char tempStr[1024];
		fscanf(inFile, "%s", tempStr);
		if (feof(inFile))
		{
			MessageBox(NULL, "Reached End of File and string \"element face\" not found!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}

		/* change tempStr to lower case */
		ChangeStrToLower(tempStr);

		if (bElementFound && !strncmp(tempStr, "face", 4))
		{
			break;
		}

		if (!strncmp(tempStr, "element", 7))
		{
			bElementFound = true;
			continue;
		}
	}

	fscanf(inFile, "%d", &nTris);
	if (feof(inFile))
	{
		MessageBox(NULL, TEXT("Reached End of File before list of vertices found!\n"),
			NULL, MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool Mesh::readOsgbNode(osg::Node* node) {
    if (!node) return false;

    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geometry = geode->getDrawable(i)->asGeometry();
            if (geometry) {
                osg::Vec3Array* verticesArray = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
				long lVertNum = verticesArray->size();
                if (verticesArray) {
                    for (unsigned int j = 0; j < verticesArray->size(); ++j) {
                        osg::Vec3 vertexOSG = verticesArray->at(j);
						
                        vertex ver(vertexOSG.x(), vertexOSG.y(), vertexOSG.z());
                        ver.setIndex(_numVerts++);
                        
                        osg::Vec3Array* normalsArray = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
						osg::Vec3 normal = normalsArray->at(j);
						Vec3 vn(normal.x(), normal.y(), normal.z());
						ver.setVertNomal(vn);

                        osg::Vec2Array* texCoordsArray = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
						osg::Vec2 texCoord = texCoordsArray->at(j);
						Vec2 uv(texCoord.x(), texCoord.y());
						ver.setUV(uv);

                        _vlist.push_back(ver);
                    }
                }
            }
			cout<< geometry->getNumPrimitiveSets()<<endl;

			int numP = geometry->getNumPrimitiveSets();
			for (int ipr = 0; ipr < numP; ipr++)
			{
				osg::PrimitiveSet* prset = geometry->getPrimitiveSet(ipr);
				auto type = prset->getType();
				switch (type) {
					case osg::PrimitiveSet::POINTS:
						std::cout << "Points" << std::endl;
						break;
					case osg::PrimitiveSet::LINES:
						std::cout << "Lines" << std::endl;
						break;
					case osg::PrimitiveSet::LINE_STRIP:
						std::cout << "Line Strip" << std::endl;
						break;
					case osg::PrimitiveSet::LINE_LOOP:
						std::cout << "Line Loop" << std::endl;
						break;
					case osg::PrimitiveSet::TRIANGLES:
						std::cout << "Triangles" << std::endl;
						break;
					case osg::PrimitiveSet::TRIANGLE_STRIP:
						std::cout << "Triangle Strip" << std::endl;
						break;
					case osg::PrimitiveSet::TRIANGLE_FAN:
						std::cout << "Triangle Fan" << std::endl;
						break;
					case osg::PrimitiveSet::QUADS:
						std::cout << "Quads" << std::endl;
						break;
					case osg::PrimitiveSet::QUAD_STRIP:
						std::cout << "Quad Strip" << std::endl;
						break;
					case osg::PrimitiveSet::POLYGON:
						std::cout << "Polygon" << std::endl;
						break;
					default:
						std::cout << "Unknown" << std::endl;
						break;
				}
				cout<<type<<endl;
				unsigned int ncnt = prset->getNumIndices();
				for (unsigned int ic = 0; ic * 3 < prset->getNumIndices(); ic++)
				{
					unsigned int v1 = prset->index(ic * 3);
					unsigned int v2 = prset->index(ic * 3 + 1);
					unsigned int v3 = prset->index(ic * 3 + 2);
					triangle t(this,v1,v2,v3);
					t.setIndex(_numTriangles);
					_plist.push_back(t);

					_vlist[v1].addTriNeighbor(_numTriangles);
					_vlist[v1].addVertNeighbor(v2);
					_vlist[v1].addVertNeighbor(v3);

					_vlist[v2].addTriNeighbor(_numTriangles);
					_vlist[v2].addVertNeighbor(v1);
					_vlist[v2].addVertNeighbor(v3);

					_vlist[v3].addTriNeighbor(_numTriangles);
					_vlist[v3].addVertNeighbor(v1);
					_vlist[v3].addVertNeighbor(v2);
					++_numTriangles;
				}
			}
        }
    }

    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            readOsgbNode(group->getChild(i));
        }
    }
	return true;
}

// Helper function for reading PLY mesh file�������ַ�ply
bool Mesh::readPlyHeader(FILE *&inFile)
{
	char tempStr[1024];

	// Read "ply" string
	do
	{
		fscanf(inFile, "%s", tempStr);
		if (feof(inFile))
		{
			MessageBox(NULL, "Reached End of File and the string \"ply\" NOT FOUND!!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}
		ChangeStrToLower(tempStr); // change tempStr to lower case 
	} while (strncmp(tempStr, "ply", 3));

	// Read # of verts
	if (!readNumPlyVerts(inFile, _numVerts))
	{
		return false;
	}

	// Read # of triangles
	if (!readNumPlyTris(inFile, _numTriangles))
	{
		return false;
	}

	// get end_header,��ȡ�ļ�������־
	do
	{
		fscanf(inFile, "%s", tempStr);
		if (feof(inFile))
		{
			MessageBox(NULL, TEXT("Reached End of File and string \"end_header\" not found!\n"),
				NULL, MB_ICONEXCLAMATION);
			return false;
		}

		/* change tempStr to lower case */
		ChangeStrToLower(tempStr);
	} while (strncmp(tempStr, "end_header", 10));

	////////// end of header
	return true;
}

// Helper function for reading PLY mesh file//���붥��ֵ�����붥������
bool Mesh::readPlyVerts(FILE *&inFile)
{
	int i;
	// read vertices
	for ( i = 0; i < _numVerts; i++)
	{
		char tempStr[1024];

#pragma warning(disable:4244)		/* disable double -> float warning */
		fscanf(inFile, "%s", tempStr);
		float x = atof(tempStr); 
		fscanf(inFile, "%s", tempStr);
		float y = atof(tempStr); 
		fscanf(inFile, "%s", tempStr);
		float z = atof(tempStr); 
#pragma warning(default:4244)		/* double -> float */

		fscanf(inFile, "%s", tempStr);
		float nx = atof(tempStr);
		fscanf(inFile, "%s", tempStr);
		float ny = atof(tempStr);
		fscanf(inFile, "%s", tempStr);
		float nz = atof(tempStr);

		fscanf(inFile, "%s", tempStr);
		float u = atof(tempStr);
		fscanf(inFile, "%s", tempStr);
		float v = atof(tempStr);


		vertex ver(x, y, z);//����vertex����󲢵��ù��캯��vertex(float x,float y,float z)���г�ʼ��
		ver.setIndex(i);//���ö���v��setIndex����
		Vec3 vn(nx, ny, nz);
		ver.setVertNomal(vn);
		Vec2 uv(u, v);
		ver.setUV(uv);

		_vlist.push_back(ver); // push_back puts a *copy* of the element at the end of the list
		if (feof(inFile))
		{
			MessageBox(NULL,"Reached End of File before all vertices found!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}

		// read until end of line
		while (fgetc(inFile) != '\n');
	}
	return true;
}

bool Mesh::writePlyFile(const std::string& filePath) const {
	std::ofstream outFile(filePath);
	int triangles_num = 0;
	if (!outFile.is_open()) {
		return false;
	}

	for (const auto& triangle : _plist) {
		if (triangle.isActive())
			++triangles_num;
	}

	outFile << "ply" << std::endl;
	outFile << "format ascii 1.0" << std::endl;
	outFile << "comment Exported by Aspose.3D 23.12.0" << std::endl;
	outFile << "element vertex " << _numVerts << std::endl;
	outFile << "property float x" << std::endl;
	outFile << "property float y" << std::endl;
	outFile << "property float z" << std::endl;
	outFile << "property float nx" << std::endl;
	outFile << "property float ny" << std::endl;
	outFile << "property float nz" << std::endl;
	outFile << "property float u" << std::endl;
	outFile << "property float v" << std::endl;
	outFile << "element face " << triangles_num << std::endl;
	outFile << "property list uchar int vertex_index" << std::endl;
	outFile << "end_header" << std::endl;

	for (const auto& vertex : _vlist) {
		outFile << vertex.getXYZ().x << " " << vertex.getXYZ().y << " " << vertex.getXYZ().z << " "
			<< vertex.getVertNorms().x << " " << vertex.getVertNorms().y << " " << vertex.getVertNorms().z << " "
			<< vertex.getUV().x << " " << vertex.getUV().y << std::endl;
	}

	for (const auto& triangle : _plist) {
		int v1, v2, v3;
		v1 = triangle.getVert1Index();
		v2 = triangle.getVert2Index();
		v3 = triangle.getVert3Index();
		if(triangle.isActive())
			outFile << "3 " << v1 << " " << v2 << " " << v3 << std::endl;
	}

	return true;
}

bool Mesh::writeOsgbFile(const std::string& filePath) const{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    for (const auto& vertex : _vlist) {
		auto xyz = vertex.getXYZ();
        vertices->push_back(osg::Vec3(xyz.x, xyz.y, xyz.z));
		auto normal = vertex.getVertNorms();
		normals->push_back(osg::Vec3(normal.x,normal.y,normal.z));
		auto uv = vertex.getUV();
		texcoords->push_back(osg::Vec2(uv.x,uv.y));
    }
    geometry->setVertexArray(vertices);
    geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
    geometry->setTexCoordArray(0, texcoords);

    osg::ref_ptr<osg::DrawElementsUInt> triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
    for (const auto& triangle : _plist) {
		if(triangle.isActive()){
			triangles->push_back(triangle.getVert1Index());
			triangles->push_back(triangle.getVert2Index());
			triangles->push_back(triangle.getVert3Index());
		}
    }
    geometry->addPrimitiveSet(triangles);

    // 创建 Geode 对象并将 Geometry 添加进去
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry);

    // 写入 osgb 文件
    bool success = osgDB::writeNodeFile(*geode, filePath);

    if (success) {
        std::cout << "Model exported to "<< filePath << std::endl;
    } else {
        std::cerr << "Failed to export model." << std::endl;
    }
	return true;
}

bool Mesh::writeFile(const std::string& filePath) const{
	const string extensionName = getFileExtension(filePath.c_str());
	if(extensionName == "ply"){
		writePlyFile(filePath);
	}
	else if(extensionName == "osgb"){
		writeOsgbFile(filePath);
	}
	return true;
}

// Helper function for reading PLY mesh file
bool Mesh::readPlyTris(FILE *&inFile)
{
	int i;
	// read triangles
	for (i = 0; i < _numTriangles; i++)
	{
		int v1, v2, v3;
		int nVerts;
		fscanf(inFile, "%d", &nVerts);
		if (3 != nVerts)
		{
			MessageBox(NULL, "Error:  Ply file contains polygons which are not triangles!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}
		fscanf(inFile, "%d", &v1);   // get value for vertex A
		fscanf(inFile, "%d", &v2);   // get value for vertex B
		fscanf(inFile, "%d", &v3);   // get value for vertex C

		// make sure verts in correct range
		assert(v1 < _numVerts && v2 < _numVerts && v3 < _numVerts);

		triangle t(this, v1, v2, v3);//����triangle�����t�����ù��캯��triangle(mesh *mp,int v1,int v2,int v3)
		t.setIndex(i);
		
		_plist.push_back(t); // push_back puts a *copy* of the element at the end of the list

		// update each vertex w/ its neighbors (vertrices & triangles)
		_vlist[v1].addTriNeighbor(i);
		_vlist[v1].addVertNeighbor(v2);
		_vlist[v1].addVertNeighbor(v3);

		_vlist[v2].addTriNeighbor(i);
		_vlist[v2].addVertNeighbor(v1);
		_vlist[v2].addVertNeighbor(v3);

		_vlist[v3].addTriNeighbor(i);
		_vlist[v3].addVertNeighbor(v1);
		_vlist[v3].addVertNeighbor(v2);

		if (feof(inFile))
		{
			MessageBox(NULL, "Reached End of File before all faces found!\n",
				NULL, MB_ICONEXCLAMATION);
			return false;
		}
		// read until end of line
		while (fgetc(inFile) != '\n');
	}
	return true;
}


// Load mesh from PLY file
bool Mesh::loadFromFile(char* filename)
{
	const string extension = getFileExtension(filename);
	if (extension == "ply") {
		FILE* inFile = fopen(filename, "rt");
		if (inFile == NULL)
		{
			char pszError[_MAX_FNAME + 1];
			sprintf(pszError, "%s does not exist!\n", filename);
			MessageBox(NULL, pszError, NULL, MB_ICONEXCLAMATION);
			return FALSE;
		}

		// read header to PLY file
		if (!readPlyHeader(inFile))
		{
			return false;
		}

		// read vertex data from PLY file
		if (!readPlyVerts(inFile))
		{
			return false;
		}

		// read triangle data from PLY file
		if (!readPlyTris(inFile))
		{
			return false;
		}

		fclose(inFile); // close the file		
	}
	else if (extension == "osgb") {
		osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(filename);
		if (!loadedModel) {
			std::cerr << "Error: Failed to load osgb file " << filename << std::endl;
			return 1;
		}
		readOsgbNode(loadedModel);
	}
	//calcVertNormals();
	return true;
}


// Recalculate the normal for one vertex
void Mesh::calcOneVertNormal(unsigned vert)
{
	vertex& v = getVertex(vert);
	const set<int>& triset = v.getTriNeighbors();

	set<int>::const_iterator iter;

	Vec3 vec;

	for (iter = triset.begin(); iter != triset.end(); ++iter)
	{
		// get the triangles for each vertex & add up the normals.
		vec += getTri(*iter).getNormalVec3();
	}

	vec.normalize(); // normalize the vertex	
	v.setVertNomal(vec);
}


// Calculate the vertex normals after loading the mesh.
void Mesh::calcVertNormals()
{
	// Iterate through the vertices
	for (unsigned i = 0; i < _vlist.size(); ++i)
	{
		calcOneVertNormal(i);
	}
}


// Used for debugging
void Mesh::dump()
{
	std::cout << "*** Mesh Dump ***" << std::endl;
	std::cout << "# of vertices: " << _numVerts << std::endl;
	std::cout << "# of triangles: " << _numTriangles << std::endl;
	for (unsigned i = 0; i < _vlist.size(); ++i)
	{
		std::cout << "\tVertex " << i << ": " << _vlist[i] << std::endl;
	}
	std::cout << std::endl;
	for (unsigned i = 0; i < _plist.size(); ++i)
	{
		std::cout << "\tTriangle " << i << ": " << _plist[i] << std::endl;
	}
	std::cout << "*** End of Mesh Dump ***" << std::endl;
	std::cout << std::endl;
}

// Get min, max values of all verts
void Mesh::setMinMax(float min[3], float max[3])
{
	max[0] = max[1] = max[2] = -FLT_MAX;
	min[0] = min[1] = min[2] = FLT_MAX;

	for (unsigned int i = 0; i < _vlist.size(); ++i)
	{
		const float* pVert = _vlist[i].getArrayVerts();
		if (pVert[0] < min[0]) min[0] = pVert[0];
		if (pVert[1] < min[1]) min[1] = pVert[1];
		if (pVert[2] < min[2]) min[2] = pVert[2];
		if (pVert[0] > max[0]) max[0] = pVert[0];
		if (pVert[1] > max[1]) max[1] = pVert[1];
		if (pVert[2] > max[2]) max[2] = pVert[2];
	}
}

// Center mesh around origin.
// Fit mesh in box from (-1, -1, -1) to (1, 1, 1)
void Mesh::Normalize()  
{
	float min[3], max[3], Scale;

	setMinMax(min, max);

	Vec3 minv(min);
	Vec3 maxv(max);

	Vec3 dimv = maxv - minv;
	
	if (dimv.x >= dimv.y && dimv.x >= dimv.z) Scale = 2.0f/dimv.x;
	else if (dimv.y >= dimv.x && dimv.y >= dimv.z) Scale = 2.0f/dimv.y;
	else Scale = 2.0f/dimv.z;

	Vec3 transv = minv + maxv;

	transv *= 0.5f;

	for (unsigned int i = 0; i < _vlist.size(); ++i)
	{
		_vlist[i].getXYZ() -= transv;
		_vlist[i].getXYZ() *= Scale;
	}
}

