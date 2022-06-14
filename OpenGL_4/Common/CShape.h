#ifndef CSHAPE_H
#define CSHAPE_H
#include "../Header/Angel.h"
#include "TypeDefine.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


// 當模型有執行 non-uniform scale 的操作時，必須透過計算反矩陣來得到正確的 Normal 方向
// 開啟以下的定義即可，目前 CPU 計算的有提供
// GPU 的部分則是設定成註解

// #define GENERAL_CASE 1 

enum class ShadingMode
{
	FLAT_SHADING_CPU = 1,		// Flat Shading 只能透過 CPU 來計算每一個面的顏色來實現，物件的預設上色方式
	GOURAUD_SHADING_CPU = 2,	// Gouraud Shading 的點頂顏色是以 CPU 來計算其顏色
	GOURAUD_SHADING_GPU = 3,	// Gouraud Shading 的點頂顏色是以 GPU 來計算其顏色
	PHONG_SHADING = 4			// Phone Shading 只能透過 GPU 在 Pixel Shader 計算每一個 Pixel 的顏色來實現
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
	point4  _vLightInView;	 // 光源在世界座標的位置
	GLuint  _uiLightInView;	 // 光源在 shader 的位置
	GLuint  _uiAmbient;		 // light's ambient  與 Object's ambient  與 ka 的乘積
	GLuint  _uiDiffuse;		 // light's diffuse  與 Object's diffuse  與 kd 的乘積
	GLuint  _uiSpecular;	 // light's specular 與 Object's specular 與 ks 的乘積
	GLuint  _uiShininess;
	GLuint  _uiLighting;

	LightSource _Light1;

	color4 _AmbientProduct;
	color4 _DiffuseProduct;
	color4 _SpecularProduct;
	int    _iLighting;	// 設定是否要打燈
//#endif

	// For Matrices
	mat4    _mxView, _mxProjection, _mxTRS;
	mat4    _mxMVFinal;
	mat3    _mxMV3X3Final, _mxITMV;	// 使用在計算 物體旋轉後的新 Normal
	mat3	_mxITView;		// View Matrix 的 Inverse Transport 
	bool    _bProjUpdated, _bViewUpdated, _bTRSUpdated;

	// For materials
	Material _Material;

	void		createBufferObject();
	void		drawingSetShader();
	void		drawingWithoutSetShader();

public:

	// For Shading Mode
	// 0: Flat shading, 1: Gouraud shading, 0 for default
	// 要變更上色模式，利用 setShadingMode 來改變
	ShadingMode _iMode;

	CShape();
	virtual ~CShape();
	virtual void draw() = 0;
	virtual void drawW() = 0; // Drawing without setting shaders
	virtual void update(float dt, point4 vLightPos, color4 vLightI) = 0;
	virtual	void update(float dt, const LightSource &Lights) = 0;
	virtual void update(float dt) = 0; // 呼叫沒有給光源的 update 代表該物件不會進行光源照明的計算

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

	// 顏色的計算以三角面為基礎，並假設一個頂點、顏色、法向量與貼圖座標都是一對一對應
	void renderWithFlatShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	void renderWithGouraudShading(point4 vLightPos, color4 vLightI);//  vLightI: Light Intensity
	void renderWithFlatShading(const LightSource& Lights);//  vLightI: Light Intensity
	void renderWithGouraudShading(const LightSource& Lights);//  vLightI: Light Intensity

//#ifdef LIGHTING_WITHGPU
	void setLightingDisable() {_iLighting = 0;}
//#endif

};

#endif
