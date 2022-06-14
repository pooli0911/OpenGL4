#include "CSolidCube.h"

CSolidCube::CSolidCube()
{
	_iNumVtx = SOLIDCUBE_NUM;
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

void CSolidCube::Quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = _vertices[b] - _vertices[a];
    vec4 v = _vertices[c] - _vertices[b];
    vec3 normal = normalize( cross(u, v) );

    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[b]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[d]; _iIndex++;
}

void CSolidCube::draw()
{
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Change to wireframe mode
	drawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, SOLIDCUBE_NUM );
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Return to solid mode
}

void CSolidCube::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, SOLIDCUBE_NUM );
}

// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CSolidCube::update(float dt, point4 vLightPos, color4 vLightI)
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

void CSolidCube::update(float dt, const LightSource& Lights)
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
void CSolidCube::update(float dt)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算
}



//
//void CSolidCube::renderWithFlatShading(vec4 vLightPos, color4 vLightI)
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
//void CSolidCube::renderWithFlatShading(const LightSource &Lights)
//{
//	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
//	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
//	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal
//
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
//void CSolidCube::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	vec4 vCentroidP;
//	for( int i = 0 ; i < _iNumVtx ; i += 6 ) {
//		_pColors[i] = _pColors[i + 3] = PhongReflectionModel(_pPoints[i], _pNormals[i], vLightPos, vLightI);
//		_pColors[i + 2] = _pColors[i + 4] = PhongReflectionModel(_pPoints[i + 2], _pNormals[i + 2], vLightPos, vLightI);
//		_pColors[i + 1] = PhongReflectionModel(_pPoints[i + 1], _pNormals[i + 1], vLightPos, vLightI);
//		_pColors[i + 5] = PhongReflectionModel(_pPoints[i + 5], _pNormals[i + 5], vLightPos, vLightI);
//	}
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
//
//void CSolidCube::renderWithGouraudShading(const LightSource &Lights)
//{
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 6) {
//		_pColors[i] = _pColors[i + 3] = PhongReflectionModel(_pPoints[i], _pNormals[i], Lights);
//		_pColors[i + 2] = _pColors[i + 4] = PhongReflectionModel(_pPoints[i + 2], _pNormals[i + 2], Lights);
//		_pColors[i + 1] = PhongReflectionModel(_pPoints[i + 1], _pNormals[i + 1], Lights);
//		_pColors[i + 5] = PhongReflectionModel(_pPoints[i + 5], _pNormals[i + 5], Lights);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}