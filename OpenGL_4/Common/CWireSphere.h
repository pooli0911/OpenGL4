#ifndef CWIRESPHERE_H
#define CWIRESPHERE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define WIRESPHERE_NUM 36  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CWireSphere : public CShape
{
private:
	GLfloat _fRadius;
	GLint _iSlices, _iStacks;

	vec4 _Points[WIRESPHERE_NUM];
	vec3 _Normals[WIRESPHERE_NUM];
	vec4 _vertices[8];
	int  _iIndex;

public:
	CWireSphere(const GLfloat fRadius=1.0f, const int iSlices=12,const  int iStacks = 6);
	~CWireSphere();

	void update(float dt, point4 vLightPos, color4 vLightI);
	void update(float dt, const LightSource &Lights);
	void update(float dt); // 不計算光源的照明

	void draw();
	void drawW(); // 呼叫不再次設定 Shader 的描繪方式

	// 顏色的計算全部交給父類別 CShape 處理，全部以三角面為單位進行計算
	//void renderWithFlatShading(point4 vLightPos, color4 vLightI);
	//void renderWithGouraudShading(point4 vLightPos, color4 vLightI);
};

#endif