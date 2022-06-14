#include "CShape.h"
#include "CShaderPool.h"

CShape::CShape()
{
	_bTRSUpdated = _bViewUpdated = _bProjUpdated = false;
	// ambient 預設為 0, diffuse, specular 的顏色都是灰色 0.5
	// Ka = 0 係數 , kd = 0.8 , ks = 0.2
	_Material.ambient = vec4(vec3(0));
	_Material.diffuse = vec4(vec3(0.5f));
	_Material.specular = vec4(vec3(0.5f));
	_Material.ka = 0; _Material.kd = 0.8f; _Material.ks = 0.2f;
	_Material.shininess = 2.0f;

	_iMode = ShadingMode::FLAT_SHADING_CPU; // 預設為 Flat Shading

	// 預設為 RGBA 為 (0.5,0.5,0.5,1.0) , 由這個灰階顏色來代表的物件顏色
	_fColor[0] = 0.5f; _fColor[1] = 0.5f; _fColor[2] = 0.5f; _fColor[3] = 1.0f;

	_iLighting = 1; // 預設接受燈光的照明
}

CShape::~CShape()
{
	if( _pPoints  != NULL ) delete [] _pPoints;  
	if( _pNormals != NULL ) delete	[] _pNormals;
	if( _pColors  != NULL ) delete	[] _pColors;
	if( _pTex != NULL ) delete	_pTex;

	if( _pVXshader != NULL ) delete [] _pVXshader;
	if( _pFSshader != NULL ) delete [] _pFSshader;
}

void CShape::createBufferObject()
{
    glGenVertexArrays( 1, &_uiVao );
    glBindVertexArray( _uiVao );

    // Create and initialize a buffer object
    glGenBuffers( 1, &_uiBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx + sizeof(vec4)*_iNumVtx, NULL, GL_STATIC_DRAW );
	// sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx + sizeof(vec4)*_iNumVtx <- vertices, normal and color

    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec4)*_iNumVtx, _pPoints );  // vertices
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx, sizeof(vec3)*_iNumVtx, _pNormals ); // // vertices' normal
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CShape::setShader(GLuint uiShaderHandle)
{
	// 改放置這裡, 方便每一個物件的設定
	createBufferObject();

	if (_iMode == ShadingMode::FLAT_SHADING_CPU || _iMode == ShadingMode::GOURAUD_SHADING_CPU)  // CPU 計算 Vertex 顏色
		setShaderName("vsLighting_CPU.glsl", "fsLighting_CPU.glsl");
	else if (_iMode == ShadingMode::GOURAUD_SHADING_GPU)
		setShaderName("vsLighting_GPU.glsl", "fsLighting_GPU.glsl"); // GPU 在 Vertex Shader 端計算頂點顏色
	else setShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");  // GPU 在 Pixel Shader 端計算每一個 Pixel 的顏色

	// Load shaders and use the resulting shader program
	if (uiShaderHandle == MAX_UNSIGNED_INT) {
		_uiProgram = CShaderPool::getInstance()->getShaderID(_pVXshader, _pFSshader);
		//_uiProgram = InitShader(_pVXshader, _pFSshader);
	}
	else _uiProgram = uiShaderHandle;

	glUseProgram(_uiProgram);
	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(_uiProgram, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(_uiProgram, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4)*_iNumVtx));

	GLuint vColorVtx = glGetAttribLocation(_uiProgram, "vVtxColor");  // vertices' color 
	glEnableVertexAttribArray(vColorVtx);
	glVertexAttribPointer(vColorVtx, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx));

	_uiModelView = glGetUniformLocation(_uiProgram, "ModelView");
	// _mxMVFinal , _mxModelView 宣告時就是單位矩陣
	glUniformMatrix4fv(_uiModelView, 1, GL_TRUE, _mxMVFinal);

	_uiProjection = glGetUniformLocation(_uiProgram, "Projection");
	// _mxProjection 宣告時就是單位矩陣
	glUniformMatrix4fv(_uiProjection, 1, GL_TRUE, _mxProjection);

	_uiColor = glGetUniformLocation(_uiProgram, "vObjectColor");
	glUniform4fv(_uiColor, 1, _fColor);

	if (_iMode == ShadingMode::GOURAUD_SHADING_GPU || _iMode == ShadingMode::PHONG_SHADING) {
		_uiLightInView = glGetUniformLocation(_uiProgram, "LightInView");
		glUniform4fv(_uiLightInView, 1, _vLightInView);

		_uiAmbient = glGetUniformLocation(_uiProgram, "AmbientProduct");
		glUniform4fv(_uiAmbient, 1, _AmbientProduct);

		_uiDiffuse = glGetUniformLocation(_uiProgram, "DiffuseProduct");
		glUniform4fv(_uiDiffuse, 1, _DiffuseProduct);

		_uiSpecular = glGetUniformLocation(_uiProgram, "SpecularProduct");
		glUniform4fv(_uiSpecular, 1, _SpecularProduct);

		_uiShininess = glGetUniformLocation(_uiProgram, "fShininess");
		glUniform1f(_uiShininess, _Material.shininess);

		_uiLighting = glGetUniformLocation(_uiProgram, "iLighting");
		glUniform1i(_uiLighting, _iLighting);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CShape::drawingSetShader()
{
	glUseProgram(_uiProgram );
	glBindVertexArray( _uiVao );
	glUniformMatrix4fv( _uiModelView, 1, GL_TRUE, _mxMVFinal );

	if( _bProjUpdated ) {
		glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
		_bProjUpdated = false;
	}
	glUniform4fv(_uiColor, 1, _fColor);

// 將資訊更新到 Shader 中
	if (_iMode == ShadingMode::GOURAUD_SHADING_GPU || _iMode == ShadingMode::PHONG_SHADING) {
		glUniform4fv(_uiLightInView, 1, _vLightInView);
		glUniform4fv(_uiAmbient, 1, _AmbientProduct);
		glUniform4fv(_uiDiffuse, 1, _DiffuseProduct);
		glUniform4fv(_uiSpecular, 1, _SpecularProduct);
		glUniform1f(_uiShininess, _Material.shininess);
		glUniform1i(_uiLighting, _iLighting);
	}
}

// 此處預設使用前一個描繪物件所使用的 Shader
void CShape::drawingWithoutSetShader()
{
	glBindVertexArray( _uiVao );
	glUniformMatrix4fv( _uiModelView, 1, GL_TRUE, _mxMVFinal );

	if( _bProjUpdated ) {
		glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
		_bProjUpdated = false;
	}
	glUniform4fv(_uiColor, 1, _fColor);

	if (_iMode == ShadingMode::GOURAUD_SHADING_GPU || _iMode == ShadingMode::PHONG_SHADING) {
		glUniform4fv(_uiLightInView, 1, _vLightInView);
		glUniform4fv(_uiAmbient, 1, _AmbientProduct);
		glUniform4fv(_uiDiffuse, 1, _DiffuseProduct);
		glUniform4fv(_uiSpecular, 1, _SpecularProduct);
		glUniform1f(_uiShininess, _Material.shininess);
		glUniform1i(_uiLighting, _iLighting);
	}
}

void CShape::setShaderName(const char vxShader[], const char fsShader[])
{
	int len;
	len = strlen(vxShader);
	_pVXshader = new char[len+1];
	memcpy(_pVXshader, vxShader, len+1);

	len = strlen(fsShader);
	_pFSshader = new char[len+1];
	memcpy(_pFSshader, fsShader, len+1);
}

void CShape::setViewMatrix(mat4 &mat)
{
	_mxView = mat;
	_bViewUpdated = true;
}

void CShape::setTRSMatrix(mat4 &mat)
{
	_mxTRS = mat;
	_bTRSUpdated = true;
}

void CShape::setProjectionMatrix(mat4 &mat)
{
	_mxProjection = mat;
	_bProjUpdated = true;
}

void CShape::setColor(vec4 vColor)
{
	_fColor[0] = vColor.x;
	_fColor[1] = vColor.y;
	_fColor[2] = vColor.z;
	_fColor[3] = vColor.w;
	//glUniform4fv(_uiColor, 1, _fColor); 
}

void CShape::setMaterials(color4 ambient, color4 diffuse, color4 specular)
{
	_Material.ambient =  ambient;
	_Material.diffuse =  diffuse;
	_Material.specular = specular;
}

void CShape::setKaKdKsShini(float ka, float kd, float ks, float shininess) // ka kd ks shininess
{
	_Material.ka = ka;
	_Material.kd = kd;
	_Material.ks = ks;
	_Material.shininess = shininess;
}

void CShape::updateMatrix()
{
	if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_bViewUpdated = _bTRSUpdated = false;
		if (_iMode == ShadingMode::FLAT_SHADING_CPU || _iMode == ShadingMode::GOURAUD_SHADING_CPU) {
			_mxMV3X3Final = mat3(
				_mxMVFinal._m[0].x, _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
				_mxMVFinal._m[0].y, _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
				_mxMVFinal._m[0].z, _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
#ifdef GENERAL_CASE
			_mxITMV = InverseTransposeMatrix(_mxMVFinal);
#endif		
		}
	}

}

// 假設每一個模型的面都是三角面，而且頂點、顏色與法向量的儲存也是三個三個一組
void CShape::renderWithFlatShading(point4 vLightPos, color4 vLightI) //  vLightI: Light Intensity
{
// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
// 計算三個頂點的 Normal 的平均值為該面的法向量

	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// 計算三角形的重心
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, vLightPos, vLightI);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void CShape::renderWithFlatShading(const LightSource& Lights) //  vLightI: Light Intensity
{
// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
// 計算三個頂點的 Normal 的平均值為該面的法向量
	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// 計算三角形的重心
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, Lights.position, Lights.diffuse);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

// 假設每一個模型的面都是三角面，而且頂點、顏色與法向量的儲存也是三個三個一組
void CShape::renderWithGouraudShading(point4 vLightPos, color4 vLightI)//  vLightI: Light Intensity
{
	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
	// 將該顏色儲存回該頂點

	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], vLightPos, vLightI);
	}

	// 如果要設定 Spot Light，在此處計算光源與被照明點的夾角，改變 vLightI 的內容，上面再呼叫傳入即可

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void  CShape::renderWithGouraudShading(const LightSource& Lights) //  vLightI: Light Intensity
{
	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
	// 將該顏色儲存回該頂點
	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], Lights);
	}
	// 如果要設定 Spot Light，在此處計算光源與被照明點的夾角，改變 vLightI 的內容，上面再呼叫傳入即可

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

// Phong Reflection Model
vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI)
{
	vec3 vN;		// 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vec4 vColor;	// 該頂點的顏色
	vec4 vPosInView, vLightInView;

	// 1. 將點頂轉到鏡頭座標系統，因為頂點會根據 TRS Matrix 移動，所以必須以 _mxMVFinal 計算在世界座標的絕對位置
	vPosInView = _mxMVFinal * vPoint;

	// 2. 將面的 Normal 轉到鏡頭座標系統，並轉成單位向量
	//    只有物件的 rotation 會改變 Normal 的方向，乘上物件的 Final ModelView Matrix，同時轉換到鏡頭座標
	//    _mxMV3X3Final 在 Update 處取自 _mxMVFinal 的左上的 3X3 部分, 不需要位移量
#ifdef GENERAL_CASE
	//	  _mxITMV 已經在呼叫 setModelViewMatrix(mat4 &mat) 與 setTRSMatrix(mat4 &mat) 就同時計算
	//    此處就不用再重新計算
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. 計算 Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * vLightI;

	// 4. 計算 Light 在鏡頭座標系統的位置
	//    此處假設 Light 的位置已經在世界座標的絕對位置
	vLightInView = _mxView * vLightPos; 

	vec3 vLight; // 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vLight.x = vLightInView.x - vPosInView.x;   
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	// 5. 計算 L dot N
	GLfloat fLdotN = vLight.x*vN.x + vLight.y*vN.y + vLight.z*vN.z;
	if( fLdotN > 0 ) { // 該點被光源照到才需要計算
		// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
		vColor +=  _Material.kd * _Material.diffuse * vLightI * fLdotN; 

		// Specular color
		// Method 1: Phone Model
		//   計算 View Vector
		vec3 vView;	
		vView.x = 0 - vPosInView.x;  // 目前已經以鏡頭座標為基礎, 所以 View 的位置就是 (0,0,0)
		vView.y = 0 - vPosInView.y;
		vView.z = 0 - vPosInView.z;
		vView = normalize(vView);

		//	 計算 Light 的 反射角 vRefL
		vec3 vRefL = 2.0f * (fLdotN) * vN - vLight;

		vRefL = normalize(vRefL);
		//   計算  vReflectedL dot View
		GLfloat RdotV = vRefL.x*vView.x + vRefL.y*vView.y + vRefL.z*vView.z;

		// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
		if( RdotV > 0 ) vColor += _Material.ks * _Material.specular * vLightI * powf(RdotV, _Material.shininess); 
	}

	vColor.w = 1; // Alpha 改設定成 1，預設都是不透明物體
	// Method 2: Modified Phone Model 

	// 將顏色儲存到  _Colors 中，因為 Quad 是兩個共平面的三角面所構成, 所以設定兩個三角面都有相同的顏色
	// 也就是設定所有的頂點都是這個顏色
	return vColor;
}

vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource &Lights)
{
	vec3 vN, vLDir;		// 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vec4 vColor;	// 該頂點的顏色
	vec4 vPosInView, vLightInView;
	

	// 1. 將點頂轉到鏡頭座標系統，因為頂點會根據 TRS Matrix 移動，所以必須以 _mxMVFinal 計算在世界座標的絕對位置
	vPosInView = _mxMVFinal * vPoint;

	// 2. 將面的 Normal 轉到鏡頭座標系統，並轉成單位向量
	//    只有物件的 rotation 會改變 Normal 的方向，乘上物件的 Final ModelView Matrix，同時轉換到鏡頭座標
	//    _mxMV3X3Final 在 Update 處取自 _mxMVFinal 的左上的 3X3 部分, 不需要位移量
#ifdef GENERAL_CASE
	//	  _mxITMV 已經在呼叫 setModelViewMatrix(mat4 &mat) 與 setTRSMatrix(mat4 &mat) 就同時計算
	//    此處就不用再重新計算
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. 計算 Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * Lights.diffuse;

	// 4. 計算 Light 在鏡頭座標系統的位置
	//    此處假設 Light 的位置已經在世界座標的絕對位置
	vLightInView = _mxView * Lights.position;

	vec3 vLight; // 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vLight.x = vLightInView.x - vPosInView.x;
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	if (Lights.type == LightType::OMNI_LIGHT) {

		// 5. 計算 L dot N
		GLfloat fLdotN = vLight.x * vN.x + vLight.y * vN.y + vLight.z * vN.z;

		if (fLdotN > 0) { // 該點被光源照到才需要計算，而且在聚光燈的範圍內
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			vColor += _Material.kd * _Material.diffuse * Lights.diffuse * fLdotN;

			// Specular color
			// Method 1: Phone Model
			//   計算 View Vector
			vec3 vView;
			vView.x = 0 - vPosInView.x;  // 目前已經以鏡頭座標為基礎, 所以 View 的位置就是 (0,0,0)
			vView.y = 0 - vPosInView.y;
			vView.z = 0 - vPosInView.z;
			vView = normalize(vView);

			//	 計算 Light 的 反射角 vRefL
			vec3 vRefL = 2.0f * (fLdotN)*vN - vLight;

			vRefL = normalize(vRefL);
			//   計算  vReflectedL dot View
			GLfloat RdotV = vRefL.x * vView.x + vRefL.y * vView.y + vRefL.z * vView.z;

			// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
			if (RdotV > 0) vColor += _Material.ks * _Material.specular * Lights.diffuse * powf(RdotV, _Material.shininess);
		}
	}
	else {  // 為 Spot Light
		vec3 vLightDir = _mxMV3X3Final * Lights.spotDirection;

		// 5. 計算照明的點是否落在 spotCutoff 之內
		GLfloat fLdotLDir = -(vLight.x * vLightDir.x + vLight.y * vLightDir.y + vLight.z * vLightDir.z);

		if (fLdotLDir >= Lights.spotCosCutoff)
		{
			// 5. 計算 L dot N
			GLfloat fLdotN = vLight.x * vN.x + vLight.y * vN.y + vLight.z * vN.z;
			color4 fLightI = Lights.diffuse * powf(fLdotLDir, Lights.spotExponent);

			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			vColor += _Material.kd * _Material.diffuse * fLightI * fLdotN;

			// Specular color
			// Method 1: Phone Model
			//   計算 View Vector
			vec3 vView;
			vView.x = 0 - vPosInView.x;  // 目前已經以鏡頭座標為基礎, 所以 View 的位置就是 (0,0,0)
			vView.y = 0 - vPosInView.y;
			vView.z = 0 - vPosInView.z;
			vView = normalize(vView);

			//	 計算 Light 的 反射角 vRefL
			vec3 vRefL = 2.0f * (fLdotN)*vN - vLight;

			vRefL = normalize(vRefL);
			//   計算  vReflectedL dot View
			GLfloat RdotV = vRefL.x * vView.x + vRefL.y * vView.y + vRefL.z * vView.z;

			// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
			if (RdotV > 0) vColor += _Material.ks * _Material.specular * fLightI * powf(RdotV, _Material.shininess);
		}
	}

	vColor.w = 1; // Alpha 改設定成 1，預設都是不透明物體
	// Method 2: Modified Phone Model 

	// 將顏色儲存到  _Colors 中，因為 Quad 是兩個共平面的三角面所構成, 所以設定兩個三角面都有相同的顏色
	// 也就是設定所有的頂點都是這個顏色
	return vColor;
}