#ifndef CWIRECUBE_H
#define CWIRECUBE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define WIRECUBE_NUM 24  // 6 faces, 4 vertices/face

class CWireCube : public CShape
{
private:
	vec4 _vertices[8];
	int  _iIndex;

	void Quad( int a, int b, int c, int d );
public:
	CWireCube();
	~CWireCube(){};

	void update(float dt, point4 vLightPos, color4 vLightI);
	void update(float dt, const LightSource &Lights);
	void update(float dt); // 不計算光源的照明
	void draw();
	void drawW();

	// 顏色的計算全部交給父類別 CShape 處理，全部以三角面為單位進行計算
	//void renderWithFlatShading(point4 vLightPos, color4 vLightI);
	//void renderWithGouraudShading(point4 vLightPos, color4 vLightI);
};

#endif