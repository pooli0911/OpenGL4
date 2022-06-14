#ifndef CSOLIDSPHERE_H
#define CSOLIDSPHERE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define SOLIDCUBE_NUM 36  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CSolidSphere : public CShape
{
private:
	GLfloat _fRadius;
	GLint _iSlices, _iStacks;

	vec4 _Points[SOLIDCUBE_NUM];
	vec3 _Normals[SOLIDCUBE_NUM];
	vec4 _vertices[8];
	int  _iIndex;

public:
	CSolidSphere(const GLfloat fRadius=1.0f, const int iSlices=12,const  int iStacks = 6);
	~CSolidSphere();

	void update(float dt, point4 vLightPos, color4 vLightI);
	void update(float dt, const LightSource &Lights);
	void update(float dt); // ���p��������ө�

	void draw();
	void drawW(); // �I�s���A���]�w Shader ���yø�覡


	// �C�⪺�p������浹�����O CShape �B�z�A�����H�T���������i��p��
	// Sphere ��ø�s��l�ϥΦh�ժ� GL_TRIANGLE_STRIP ��ø�s, �]���S����k���� Flat Shading�A
	// �u���H vertex ����¦���p���C�⪺ Ground Shading
	//void renderWithFlatShading(point4 vLightPos, color4 vLightI);
	//void renderWithGouraudShading(point4 vLightPos, color4 vLightI);
	//void renderWithFlatShading(const LightSource &Lights);
	//void renderWithGouraudShading(const LightSource &Lights);
};

#endif