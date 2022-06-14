#include <cmath>
#include "CQuad.h"
// Example 4 �}�l
// ���P Example 3 �¤W(Y�b)
// �C�@�� Vertex �W�[ Normal �A�令�~���� CShape�A�@�ֳB�z�������]�w�ݨD

CQuad::CQuad()
{
	_iNumVtx = QUAD_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 

	_pPoints[0] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[1] = vec4(  0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[2] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[3] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[4] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[5] = vec4( -0.5f, 0.0f, -0.5f, 1.0f);

	_pNormals[0] = vec3(  0, 1.0f, 0);  // Normal Vector �� W �� 0
	_pNormals[1] = vec3(  0, 1.0f, 0);
	_pNormals[2] = vec3(  0, 1.0f, 0);
	_pNormals[3] = vec3(  0, 1.0f, 0);
	_pNormals[4] = vec3(  0, 1.0f, 0);
	_pNormals[5] = vec3(  0, 1.0f, 0);

	_pColors[0] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);  // (r, g, b, a)
	_pColors[1] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[2] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[3] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[4] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[5] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);

	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);
	
	// �]�w����
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

// ���B�ҵ��� vLightPos �����O�@�ɮy�Ъ��T�w�����m
void CQuad::update(float dt, point4 vLightPos, color4 vLightI)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(vLightPos, vLightI);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(vLightPos, vLightI);
	else {
		_vLightInView = _mxView * vLightPos;	// �N Light �ഫ�����Y�y�ЦA�ǤJ shader
		// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
		_AmbientProduct = _Material.ka * _Material.ambient * vLightI;
		_DiffuseProduct = _Material.kd * _Material.diffuse * vLightI;
		_SpecularProduct = _Material.ks * _Material.specular * vLightI;
	}
}

void CQuad::update(float dt,const LightSource &Lights)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��

	if (_iMode == ShadingMode::FLAT_SHADING_CPU ) renderWithFlatShading(Lights);
	else if ( _iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(Lights);
	else {
		_vLightInView = _mxView * Lights.position;		// �N Light �ഫ�����Y�y�ЦA�ǤJ shader
		// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
		_AmbientProduct = _Material.ka * _Material.ambient * Lights.ambient;
		_DiffuseProduct = _Material.kd * _Material.diffuse * Lights.diffuse;
		_SpecularProduct = _Material.ks * _Material.specular * Lights.specular;
	}
}

// �I�s�S���������� update �N��Ӫ��󤣷|�i������ө����p��
void CQuad::update(float dt)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��
}

void CQuad::setVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor)
{
	_pColors[3] = _pColors[0] = vLFColor;
	_pColors[1] = vLRColor;
	_pColors[4] = _pColors[2] = vTRColor;
	_pColors[5] = vTLColor;

	// �N�Ҧ� vertices �C���s�� VBO ��
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::setVtxColors(vec4 vFColor, vec4 vSColor) // �]�w��ӤT���Ϊ��C��
{
	_pColors[0] = _pColors[1] = _pColors[2] = vFColor;
	_pColors[3] = _pColors[4] = _pColors[5] = vSColor;

	// �N�Ҧ� vertices �C���s�� VBO ��
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::draw()
{
	drawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
}

void CQuad::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
}

//
//void CQuad::renderWithFlatShading(vec4 vLightPos, color4 vLightI)
//{
//	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
//	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
//	// �]���C�@�ӥ��������I�� Normal ���ۦP�A�ҥH���B�èS���p�⦹�T�ӳ��I������ Normal
//
//	vec4 vCentroidP;
//	for( int i = 0 ; i < _iNumVtx ; i += 3 ) {
//		// �p��T���Ϊ�����
//		vCentroidP = (_pPoints[i] + _pPoints[i+1] + _pPoints[i+2])/3.0f;
//		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, _pNormals[i], vLightPos, vLightI);
//	}
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//void CQuad::renderWithFlatShading(const LightSource &Lights)
//{
//	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
//	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
//	// �]���C�@�ӥ��������I�� Normal ���ۦP�A�ҥH���B�èS���p�⦹�T�ӳ��I������ Normal
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 3) {
//		// �p��T���Ϊ�����
//		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
//		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, _pNormals[i], Lights.position, Lights.diffuse);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}
//
//void CQuad::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	// �H vertex �����A�ھڸ��I����m�P��k�V�q�A�Q�� Phong lighting model �p��۹������C��
//	// �N���C���x�s�^�ӳ��I
//
//	// ���ӳ��I�c������ӤT����
//	// �p�� 0 1 2 5 �|�ӳ��I���C��Y�i�A0 �P 3�B2 �P 4 ���C��ۦP
//	_pColors[0] = _pColors[3] = PhongReflectionModel(_pPoints[0], _pNormals[0], vLightPos,  vLightI);
//	_pColors[2] = _pColors[4] = PhongReflectionModel(_pPoints[2], _pNormals[2], vLightPos,  vLightI);
//	_pColors[1] = PhongReflectionModel(_pPoints[1], _pNormals[1], vLightPos,  vLightI);
//	_pColors[5] = PhongReflectionModel(_pPoints[5], _pNormals[5], vLightPos,  vLightI);
//
//	// �p�G�n�]�w Spot Light�A�b���B�p������P�Q�ө��I�������A���� vLightI �����e�A�W���b�I�s�ǤJ�Y�i
//
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//void CQuad::renderWithGouraudShading(const LightSource &Lights)
//{
//	// �H vertex �����A�ھڸ��I����m�P��k�V�q�A�Q�� Phong lighting model �p��۹������C��
//	// �N���C���x�s�^�ӳ��I
//
//	// ���ӳ��I�c������ӤT����
//	// �p�� 0 1 2 5 �|�ӳ��I���C��Y�i�A0 �P 3�B2 �P 4 ���C��ۦP
//	_pColors[0] = _pColors[3] = PhongReflectionModel(_pPoints[0], _pNormals[0], Lights);
//	_pColors[2] = _pColors[4] = PhongReflectionModel(_pPoints[2], _pNormals[2], Lights);
//	_pColors[1] = PhongReflectionModel(_pPoints[1], _pNormals[1], Lights);
//	_pColors[5] = PhongReflectionModel(_pPoints[5], _pNormals[5], Lights);
//
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}
