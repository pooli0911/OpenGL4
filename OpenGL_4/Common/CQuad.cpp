#include <cmath>
#include "CQuad.h"
// Example 4 開始
// 面同 Example 3 朝上(Y軸)
// 每一個 Vertex 增加 Normal ，改成繼曾自 CShape，一併處理相關的設定需求

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

	_pNormals[0] = vec3(  0, 1.0f, 0);  // Normal Vector 的 W 為 0
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
	
	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CQuad::update(float dt, point4 vLightPos, color4 vLightI)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(vLightPos, vLightI);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(vLightPos, vLightI);
	else {
		_vLightInView = _mxView * vLightPos;	// 將 Light 轉換到鏡頭座標再傳入 shader
		// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
		_AmbientProduct = _Material.ka * _Material.ambient * vLightI;
		_DiffuseProduct = _Material.kd * _Material.diffuse * vLightI;
		_SpecularProduct = _Material.ks * _Material.specular * vLightI;
	}
}

void CQuad::update(float dt,const LightSource &Lights)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算

	if (_iMode == ShadingMode::FLAT_SHADING_CPU ) renderWithFlatShading(Lights);
	else if ( _iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(Lights);
	else {
		_vLightInView = _mxView * Lights.position;		// 將 Light 轉換到鏡頭座標再傳入 shader
		// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
		_AmbientProduct = _Material.ka * _Material.ambient * Lights.ambient;
		_DiffuseProduct = _Material.kd * _Material.diffuse * Lights.diffuse;
		_SpecularProduct = _Material.ks * _Material.specular * Lights.specular;
	}
}

// 呼叫沒有給光源的 update 代表該物件不會進行光源照明的計算
void CQuad::update(float dt)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算
}

void CQuad::setVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor)
{
	_pColors[3] = _pColors[0] = vLFColor;
	_pColors[1] = vLRColor;
	_pColors[4] = _pColors[2] = vTRColor;
	_pColors[5] = vTLColor;

	// 將所有 vertices 顏色更新到 VBO 中
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::setVtxColors(vec4 vFColor, vec4 vSColor) // 設定兩個三角形的顏色
{
	_pColors[0] = _pColors[1] = _pColors[2] = vFColor;
	_pColors[3] = _pColors[4] = _pColors[5] = vSColor;

	// 將所有 vertices 顏色更新到 VBO 中
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
//	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
//	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
//	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal
//
//	vec4 vCentroidP;
//	for( int i = 0 ; i < _iNumVtx ; i += 3 ) {
//		// 計算三角形的重心
//		vCentroidP = (_pPoints[i] + _pPoints[i+1] + _pPoints[i+2])/3.0f;
//		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, _pNormals[i], vLightPos, vLightI);
//	}
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//void CQuad::renderWithFlatShading(const LightSource &Lights)
//{
//	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
//	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
//	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 3) {
//		// 計算三角形的重心
//		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
//		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, _pNormals[i], Lights.position, Lights.diffuse);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}
//
//void CQuad::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
//	// 將該顏色儲存回該頂點
//
//	// 六個頂點構成的兩個三角形
//	// 計算 0 1 2 5 四個頂點的顏色即可，0 與 3、2 與 4 的顏色相同
//	_pColors[0] = _pColors[3] = PhongReflectionModel(_pPoints[0], _pNormals[0], vLightPos,  vLightI);
//	_pColors[2] = _pColors[4] = PhongReflectionModel(_pPoints[2], _pNormals[2], vLightPos,  vLightI);
//	_pColors[1] = PhongReflectionModel(_pPoints[1], _pNormals[1], vLightPos,  vLightI);
//	_pColors[5] = PhongReflectionModel(_pPoints[5], _pNormals[5], vLightPos,  vLightI);
//
//	// 如果要設定 Spot Light，在此處計算光源與被照明點的夾角，改變 vLightI 的內容，上面在呼叫傳入即可
//
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//void CQuad::renderWithGouraudShading(const LightSource &Lights)
//{
//	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
//	// 將該顏色儲存回該頂點
//
//	// 六個頂點構成的兩個三角形
//	// 計算 0 1 2 5 四個頂點的顏色即可，0 與 3、2 與 4 的顏色相同
//	_pColors[0] = _pColors[3] = PhongReflectionModel(_pPoints[0], _pNormals[0], Lights);
//	_pColors[2] = _pColors[4] = PhongReflectionModel(_pPoints[2], _pNormals[2], Lights);
//	_pColors[1] = PhongReflectionModel(_pPoints[1], _pNormals[1], Lights);
//	_pColors[5] = PhongReflectionModel(_pPoints[5], _pNormals[5], Lights);
//
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}
