#ifndef CLINSEGMENT_H
#define CLINSEGMENT_H
#include "../header/Angel.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define VERTEX_NUM 2  // one line segment with two vertices

class CLineSegment
{
private:
	vec4 _Points[VERTEX_NUM];
	vec4 _Colors[VERTEX_NUM];

	//  for shader
	mat4 _mxView, _mxProjection;
	mat4 _mxMVFinal, _mxTRS;

	GLfloat _fColor[4];
	GLuint _uiModelView, _uiProjection, _uiColor;
	GLuint _uiProgram ;
	GLuint _uiBuffer;
	bool   _bUpdateView;
	bool   _bUpdateProj;

	// For VAO
	GLuint _uiVao;
	void createBufferObject();

public:
	CLineSegment(const vec4 SPoint = vec4(-1,0,0,0), const vec4 EPoint = vec4(1,0,0,0), const vec4 vColor = vec4(1,0,0,0) );

	void setShader(mat4 &mxModelView, mat4 &mxProjection, GLuint uiShaderHandle=MAX_UNSIGNED_INT);
	void setShader(GLuint uiShaderHandle = MAX_UNSIGNED_INT);
	GLuint GetShaderHandle() { return _uiProgram;}
	void setViewMatrix(mat4 &mat);
	void setProjectionMatrix(mat4 &mat);
	void setTRSMatrix(mat4 &mat);
	void setColor(vec4 vColor); // Single color
	void setVtxColors(vec4 vSPColor, vec4 vEPColor); // Vertices' Color
	void updatePosition(vec4 SPoint, vec4 EPoint);

	void draw();
	void drawW();
};




#endif