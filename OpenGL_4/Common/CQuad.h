#ifndef CQUAD_H
#define CQUAD_H
#include "../header/Angel.h"
#include "CShape.h"

#define QUAD_NUM 6		// 2 faces, 2 triangles/face 

class CQuad : public CShape
{
private:

	//vec4 m_Points[QUAD_NUM];
	//vec3 m_Normal[QUAD_NUM];	// 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	//vec4 m_Colors[QUAD_NUM];

public:
	CQuad();

	void update(float dt, point4 vLightPos, color4 vLightI);
	void update(float dt, const LightSource &Lights);
	void update(float dt); // 不計算光源的照明

	GLuint GetShaderHandle() { return _uiProgram;} 
	void setVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor); // four Vertices' Color
	void setVtxColors(vec4 vFColor, vec4 vSColor);	// three Vertices' Color with idx as the first 

	void draw();
	void drawW();

	// 顏色的計算全部交給父類別 CShape 處理，全部以三角面為單位進行計算
	//void renderWithFlatShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	//void renderWithGouraudShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	//void renderWithFlatShading(const LightSource &Lights);//  vLightI: Light Intensity
	//void renderWithGouraudShading(const LightSource &Lights);//  vLightI: Light Intensity
};




#endif