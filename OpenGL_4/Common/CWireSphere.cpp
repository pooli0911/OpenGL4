#include "CWireSphere.h"

CWireSphere::CWireSphere(const GLfloat fRadius, const int iSlices, const  int iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;  
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;  
    GLfloat ds = 1.0f / (GLfloat) iSlices;  
    GLfloat dt = 1.0f / (GLfloat) iStacks;  
    GLfloat t = 1.0f;      
    GLfloat s = 0.0f;  
    GLint i, j;     // Looping variables  
	int idx = 0; // 儲存 vertex 順序的索引值

	_fRadius = fRadius;
	_iSlices = iSlices;
	_iStacks = iStacks;
	_iNumVtx = iStacks*(2*(iSlices+1));

	_pPoints = NULL; _pNormals = NULL; _pTex = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex     = new vec2[_iNumVtx];


	for (i = 0; i < iStacks; i++ ) {  
		GLfloat rho = (GLfloat)i * drho;  
		GLfloat srho = (GLfloat)(sin(rho));  
		GLfloat crho = (GLfloat)(cos(rho));  
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));  
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));  
		
		// Many sources of OpenGL sphere drawing code uses a triangle fan  
		// for the caps of the sphere. This however introduces texturing   
		// artifacts at the poles on some OpenGL implementations  
		s = 0.0f;  
		for ( j = 0; j <= iSlices; j++) {  
            GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;  
            GLfloat stheta = (GLfloat)(-sin(theta));  
            GLfloat ctheta = (GLfloat)(cos(theta));  
  
            GLfloat x = stheta * srho;  
            GLfloat y = ctheta * srho;  
            GLfloat z = crho;  
              
			_pPoints[idx].x = x * _fRadius;
			_pPoints[idx].y = y * _fRadius;
			_pPoints[idx].z = z * _fRadius;
			_pPoints[idx].w = 1;

			_pNormals[idx].x = x;
			_pNormals[idx].y = y;
			_pNormals[idx].z = z;

			_pTex[idx].x = s;
			_pTex[idx].y = t; // 設定貼圖座標
			idx++;

            x = stheta * srhodrho;  
            y = ctheta * srhodrho;  
            z = crhodrho;

			_pPoints[idx].x = x * _fRadius;
			_pPoints[idx].y = y * _fRadius;
			_pPoints[idx].z = z * _fRadius;
			_pPoints[idx].w = 1;

			_pNormals[idx].x = x;
			_pNormals[idx].y = y;
			_pNormals[idx].z = z;

			_pTex[idx].x = s;
			_pTex[idx].y = t - dt; // 設定貼圖座標
			idx++;
			s += ds; 
		}   
		t -= dt;  
	}  
	// 預設將所有的面都設定成灰色
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CWireSphere::update(float dt, point4 vLightPos, color4 vLightI)
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

void CWireSphere::update(float dt, const LightSource& Lights)
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
void CWireSphere::update(float dt)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算
}

void CWireSphere::draw()
{
	drawingSetShader();
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_LINE_LOOP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
}


void CWireSphere::drawW()
{
	drawingWithoutSetShader();
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_LINE_LOOP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
}

CWireSphere::~CWireSphere()
{

}

//
//// 回家自己寫
//void CWireSphere::renderWithFlatShading(vec4 vLightPos, color4 vLightI)
//{
//
//
//}
//
//
//void CWireSphere::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	// Method 1 : 對每一個 Vertex 都計算顏色
//	for (int i = 0; i < _iStacks; i++ ) {  
//		for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
//			_pColors[i*2*(_iSlices+1)+j] = PhongReflectionModel(_pPoints[i*2*(_iSlices+1)+j], _pNormals[i*2*(_iSlices+1)+j], vLightPos, vLightI);
//		}
//	}
//
//	// Method 2 : 重疊的 Vertex 使用前一次計算的顏色
//	// 先計算第一個 Stack 的顏色
//	//for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
//	//	_pColors[j] = PhongLightingModel(_pPoints[j], _pNormals[j], vLightPos, vViewPoint, vLightI);
//	//}
//	//// 後續 Stack 的 vertex 顏色，編號偶數(含 0) 使用前一個 stack 編號+1的 顏色
//	//// 編號奇數就必須計算顏色
//	//// 每一個 Slices 最後兩個 vertex 於開頭前兩個 vertex 重疊，所以使用該兩個 vertex 的顏色
//	//for (int i = 1; i < _iStacks; i++ ) {  
//	//	for( int j = 0 ; j < 2*(_iSlices+1) - 2 ; j++ ) {
//	//		if( j%2 ) _pColors[i*2*(_iSlices+1)+j] = PhongLightingModel(_pPoints[i*2*(_iSlices+1)+j], _pNormals[i*2*(_iSlices+1)+j], vLightPos, vViewPoint, vLightI);
//	//		else _pColors[i*2*(_iSlices+1)+j] =  _pColors[(i-1)*2*(_iSlices+1)+j+1];		
//	//	}
//	//	_pColors[(i+1)*2*(_iSlices+1)-2] = _pColors[i*2*(_iSlices+1)];
//	//	_pColors[(i+1)*2*(_iSlices+1)-1] = _pColors[i*2*(_iSlices+1)+1];
//	//}
//
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}
