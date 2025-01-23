#include <iostream>
#include <osg/Geometry>
#include <ostream>

struct Vertex {
    float position[3]; // 顶点位置 (x, y, z)
    float normal[3];   // 法线 (nx, ny, nz)
    float uv[2];       // UV 坐标 (u, v)

    // 构造函数，初始化默认值
    Vertex() {
        position[0] = position[1] = position[2] = 0.0f;
        normal[0] = normal[1] = normal[2] = 0.0f;
        uv[0] = uv[1] = 0.0f;
    }

    ~Vertex() = default;

        // 设置位置
    void setPosition(float x, float y, float z) {
        position[0] = x;
        position[1] = y;
        position[2] = z;
    }

    // 获取位置
    void getPosition(float &x, float &y, float &z) const {
        x = position[0];
        y = position[1];
        z = position[2];
    }

    // 设置法线
    void setNormal(float nx, float ny, float nz) {
        normal[0] = nx;
        normal[1] = ny;
        normal[2] = nz;
    }

    // 获取法线
    void getNormal(float &nx, float &ny, float &nz) const {
        nx = normal[0];
        ny = normal[1];
        nz = normal[2];
    }

    // 设置UV
    void setUV(float u, float v) {
        uv[0] = u;
        uv[1] = v;
    }

    // 获取UV
    void getUV(float &u, float &v) const {
        u = uv[0];
        v = uv[1];
    }
};
