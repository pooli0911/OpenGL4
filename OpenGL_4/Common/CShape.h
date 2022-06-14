#ifndef CSHAPE_H
#define CSHAPE_H
#include "../Header/Angel.h"
#include "TypeDefine.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


// ��ҫ������� non-uniform scale ���ާ@�ɡA�����z�L�p��ϯx�}�ӱo�쥿�T�� Normal ��V
// �}�ҥH�U���w�q�Y�i�A�ثe CPU �p�⪺������
// GPU �������h�O�]�w������

// #define GENERAL_CASE 1 

enum class ShadingMode
{
	FLAT_SHADING_CPU = 1,		// Flat Shading �u��z�L CPU �ӭp��C�@�ӭ����C��ӹ�{�A���󪺹w�]�W��覡
	GOURAUD_SHADING_CPU = 2,	// Gouraud Shading ���I���C��O�H CPU �ӭp����C��
	GOURAUD_SHADING_GPU = 3,	// Gouraud Shading ���I���C��O�H GPU �ӭp����C��
	PHONG_SHADING = 4			// Phone Shading �u��z�L GPU �b Pixel Shader �p��C�@�� Pixel ���C��ӹ�{
};

class CShape 
{
protected:
	vec4 *_pPoints;
	vec3 *_pNormals;
	vec4 *_pColors;
	vec2 *_pTex;
	int  _iNumVtx;

	GLfloat _fColor[4]; // Object's color
	// For shaders' name
	char *_pVXshader, *_pFSshader;

	// For VAO
	GLuint _uiVao;

	// For Shader
	GLuint  _uiModelView, _uiProjection, _uiColor;
	GLuint  _uiProgram;
	GLuint  _uiBuffer;

// #ifdef LIGHTING_WITHGPU
	point4  _vLightInView;	 // �����b�@�ɮy�Ъ���m
	GLuint  _uiLightInView;	 // �����b shader ����m
	GLuint  _uiAmbient;		 // light's ambient  �P Object's ambient  �P ka �����n
	GLuint  _uiDiffuse;		 // light's diffuse  �P Object's diffuse  �P kd �����n
	GLuint  _uiSpecular;	 // light's specular �P Object's specular �P ks �����n
	GLuint  _uiShininess;
	GLuint  _uiLighting;

	LightSource _Light1;

	color4 _AmbientProduct;
	color4 _DiffuseProduct;
	color4 _SpecularProduct;
	int    _iLighting;	// �]�w�O�_�n���O
//#endif

	// For Matrices
	mat4    _mxView, _mxProjection, _mxTRS;
	mat4    _mxMVFinal;
	mat3    _mxMV3X3Final, _mxITMV;	// �ϥΦb�p�� �������᪺�s Normal
	mat3	_mxITView;		// View Matrix �� Inverse Transport 
	bool    _bProjUpdated, _bViewUpdated, _bTRSUpdated;

	// For materials
	Material _Material;

	void		createBufferObject();
	void		drawingSetShader();
	void		drawingWithoutSetShader();

public:

	// For Shading Mode
	// 0: Flat shading, 1: Gouraud shading, 0 for default
	// �n�ܧ�W��Ҧ��A�Q�� setShadingMode �ӧ���
	ShadingMode _iMode;

	CShape();
	virtual ~CShape();
	virtual void draw() = 0;
	virtual void drawW() = 0; // Drawing without setting shaders
	virtual void update(float dt, point4 vLightPos, color4 vLightI) = 0;
	virtual	void update(float dt, const LightSource &Lights) = 0;
	virtual void update(float dt) = 0; // �I�s�S���������� update �N��Ӫ��󤣷|�i������ө����p��

	void updateMatrix();
	void setShaderName(const char vxShader[], const char fsShader[]);
	void setShader(GLuint uiShaderHandle = MAX_UNSIGNED_INT);
	void setColor(vec4 vColor);
	void setViewMatrix(mat4 &mat);
	void setProjectionMatrix(mat4 &mat);
	void setTRSMatrix(mat4 &mat);

	// For setting materials 
	void setMaterials(color4 ambient, color4 diffuse, color4 specular);
	void setKaKdKsShini(float ka, float kd, float ks, float shininess); // ka kd ks shininess

	// For Lighting Calculation
	void setShadingMode(ShadingMode iMode) {_iMode = iMode;}
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI);
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource& Lights);

	// �C�⪺�p��H�T��������¦�A�ð��]�@�ӳ��I�B�C��B�k�V�q�P�K�Ϯy�г��O�@��@����
	void renderWithFlatShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	void renderWithGouraudShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	void renderWithFlatShading(const LightSource& Lights);//  vLightI: Light Intensity
	void renderWithGouraudShading(const LightSource& Lights);//  vLightI: Light Intensity

//#ifdef LIGHTING_WITHGPU
	void setLightingDisable() {_iLighting = 0;}
//#endif

};

#endif
