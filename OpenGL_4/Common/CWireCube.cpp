#include "CWireCube.h"

CWireCube::CWireCube()
{

	_iNumVtx = WIRECUBE_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex     = new vec2[_iNumVtx];

    _vertices[0] = point4( -0.5, -0.5,  0.5, 1.0 );
    _vertices[1] = point4( -0.5,  0.5,  0.5, 1.0 );
    _vertices[2] = point4(  0.5,  0.5,  0.5, 1.0 );
    _vertices[3] = point4(  0.5, -0.5,  0.5, 1.0 );
    _vertices[4] = point4( -0.5, -0.5, -0.5, 1.0 );
    _vertices[5] = point4( -0.5,  0.5, -0.5, 1.0 );
    _vertices[6] = point4(  0.5,  0.5, -0.5, 1.0 );
	_vertices[7] = point4(  0.5, -0.5, -0.5, 1.0 );

	_iIndex = 0;
	// generate 12 triangles: 36 vertices and 36 colors
    Quad( 1, 0, 3, 2 );
    Quad( 2, 3, 7, 6 );
    Quad( 3, 0, 4, 7 );
    Quad( 6, 5, 1, 2 );
    Quad( 4, 5, 6, 7 );
    Quad( 5, 4, 0, 1 );

	// 預設將所有的面都設定成灰色
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

void CWireCube::Quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = _vertices[b] - _vertices[a];
    vec4 v = _vertices[c] - _vertices[b];
    vec3 normal = normalize( cross(u, v) );

    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[b]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[d]; _iIndex++;
}

// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CWireCube::update(float dt, point4 vLightPos, color4 vLightI)
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

void CWireCube::update(float dt, const LightSource& Lights)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(Lights);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(Lights);
	else {
		_vLightInView = _mxView * Lights.position;		// 將 Light 轉換到鏡頭座標再傳入 shader
		// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
		_AmbientProduct = _Material.ka * _Material.ambient * Lights.ambient;
		_DiffuseProduct = _Material.kd * _Material.diffuse * Lights.diffuse;
		_SpecularProduct = _Material.ks * _Material.specular * Lights.specular;
	}
}

// 呼叫沒有給光源的 update 代表該物件不會進行光源照明的計算
void CWireCube::update(float dt)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算
}


void CWireCube::draw()
{
	drawingSetShader();
	glDrawArrays( GL_LINE_LOOP,  0, 4 );
	glDrawArrays( GL_LINE_LOOP,  4, 4 );
	glDrawArrays( GL_LINE_LOOP,  8, 4 );
	glDrawArrays( GL_LINE_LOOP, 12, 4 );
	glDrawArrays( GL_LINE_LOOP, 16, 4 );
	glDrawArrays( GL_LINE_LOOP, 20, 4 );
}


void CWireCube::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_LINE_LOOP,  0, 4 );
	glDrawArrays( GL_LINE_LOOP,  4, 4 );
	glDrawArrays( GL_LINE_LOOP,  8, 4 );
	glDrawArrays( GL_LINE_LOOP, 12, 4 );
	glDrawArrays( GL_LINE_LOOP, 16, 4 );
	glDrawArrays( GL_LINE_LOOP, 20, 4 );
}




//
//void CWireCube::renderWithFlatShading(vec4 vLightPos, color4 vLightI)
//{
//	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
//	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
//	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal
//
//	vec4 vCentroidP;
//	for( int i = 0 ; i < _iNumVtx ; i += 4 ) {
//		// 計算三角形的重心
//		vCentroidP = (_pPoints[i] + _pPoints[i+1] + _pPoints[i+2] + _pPoints[i+3])/4.0f;
//		_pColors[i] = _pColors[i+1] = _pColors[i+2] = _pColors[i+3] = PhongReflectionModel(vCentroidP, _pNormals[i], vLightPos, vLightI);
//	}
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//
//void CWireCube::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	for( int i = 0 ; i < _iNumVtx ; i += 4 ) {
//		_pColors[i]   = PhongReflectionModel(_pPoints[i], _pNormals[i], vLightPos,  vLightI);
//		_pColors[i+1] = PhongReflectionModel(_pPoints[i+1], _pNormals[i+1], vLightPos,  vLightI);
//		_pColors[i+2] = PhongReflectionModel(_pPoints[i+2], _pNormals[i+2], vLightPos,  vLightI);
//		_pColors[i+3] = PhongReflectionModel(_pPoints[i+3], _pNormals[i+3], vLightPos,  vLightI);
//	}
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}