#include "CSolidSphere.h"

CSolidSphere::CSolidSphere(const GLfloat fRadius, const int iSlices,const  int iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;  
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;  
    GLfloat ds = 1.0f / (GLfloat) iSlices;  
    GLfloat dt = 1.0f / (GLfloat) iStacks;  
    GLfloat t = 1.0f;      
    GLfloat s = 0.0f;  
    GLint i, j;     // Looping variables  
	int idx = 0; // �x�s vertex ���Ǫ����ޭ�

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
			_pTex[idx].y = t; // �]�w�K�Ϯy��
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
			_pTex[idx].y = t - dt; // �]�w�K�Ϯy��
			idx++;
			s += ds; 
		}  
		t -= dt;  
	}  

	// �w�]�N�Ҧ��������]�w���Ǧ�
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

	// �]�w����
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

// ���B�ҵ��� vLightPos �����O�@�ɮy�Ъ��T�w�����m
void CSolidSphere::update(float dt, point4 vLightPos, color4 vLightI)
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

void CSolidSphere::update(float dt, const LightSource& Lights)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(Lights);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(Lights);
	else {
		_vLightInView = _mxView * Lights.position;		// �N Light �ഫ�����Y�y�ЦA�ǤJ shader
		// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
		_AmbientProduct = _Material.ka * _Material.ambient * Lights.ambient;
		_DiffuseProduct = _Material.kd * _Material.diffuse * Lights.diffuse;
		_SpecularProduct = _Material.ks * _Material.specular * Lights.specular;
	}
}

// �I�s�S���������� update �N��Ӫ��󤣷|�i������ө����p��
void CSolidSphere::update(float dt)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��
}

void CSolidSphere::draw()
{
	drawingSetShader();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Change to wireframe mode
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_TRIANGLE_STRIP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Return to solid mode
}

void CSolidSphere::drawW()
{
	drawingWithoutSetShader();
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_TRIANGLE_STRIP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
}

CSolidSphere::~CSolidSphere()
{

}

//
//// �^�a�ۤv�g
//void CSolidSphere::renderWithFlatShading(vec4 vLightPos, color4 vLightI)
//{
//
//
//}
//void CSolidSphere::renderWithFlatShading(const LightSource &Lights)
//{
//
//
//}
//
//void CSolidSphere::renderWithGouraudShading(const LightSource &Lights)
//{
//	// Method 1 : ��C�@�� Vertex ���p���C��
//	for (int i = 0; i < _iStacks; i++) {
//		for (int j = 0; j < 2 * (_iSlices + 1); j++) {
//			_pColors[i * 2 * (_iSlices + 1) + j] = PhongReflectionModel(_pPoints[i * 2 * (_iSlices + 1) + j], _pNormals[i * 2 * (_iSlices + 1) + j], Lights);
//		}
//	}
//
//	// Method 2 : ���|�� Vertex �ϥΫe�@���p�⪺�C��
//	//���p��Ĥ@�� Stack ���C��
//	//for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
//	//	_pColors[j] = PhongReflectionModel(_pPoints[j], _pNormals[j], vLightPos, vLightI);
//	//}
//	//// ���� Stack �� vertex �C��A�s������(�t 0) �ϥΫe�@�� stack �s��+1�� �C��
//	//// �s���_�ƴN�����p���C��
//	//// �C�@�� Slices �̫��� vertex ��}�Y�e��� vertex ���|�A�ҥH�ϥθӨ�� vertex ���C��
//	//for (int i = 1; i < _iStacks; i++ ) {  
//	//	for( int j = 0 ; j < 2*(_iSlices+1) - 2 ; j++ ) {
//	//		if( j%2 ) _pColors[i*2*(_iSlices+1)+j] = PhongReflectionModel(_pPoints[i*2*(_iSlices+1)+j], _pNormals[i*2*(_iSlices+1)+j], vLightPos, vLightI);
//	//		else _pColors[i*2*(_iSlices+1)+j] =  _pColors[(i-1)*2*(_iSlices+1)+j+1];		
//	//	}
//	//	_pColors[(i+1)*2*(_iSlices+1)-2] = _pColors[i*2*(_iSlices+1)];
//	//	_pColors[(i+1)*2*(_iSlices+1)-1] = _pColors[i*2*(_iSlices+1)+1];
//	//}
//
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors); // vertcies' Color
//}
//
//void CSolidSphere::renderWithGouraudShading(vec4 vLightPos, color4 vLightI)
//{
//	// Method 1 : ��C�@�� Vertex ���p���C��
//	 for (int i = 0; i < _iStacks; i++ ) {  
//		 for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
//			 _pColors[i * 2 * (_iSlices + 1) + j] = PhongReflectionModel(_pPoints[i * 2 * (_iSlices + 1) + j], _pNormals[i * 2 * (_iSlices + 1) + j], vLightPos, vLightI);
//		 }
//	 } 
//
//	// Method 2 : ���|�� Vertex �ϥΫe�@���p�⪺�C��
//	 //���p��Ĥ@�� Stack ���C��
//	//for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
//	//	_pColors[j] = PhongReflectionModel(_pPoints[j], _pNormals[j], vLightPos, vLightI);
//	//}
//	//// ���� Stack �� vertex �C��A�s������(�t 0) �ϥΫe�@�� stack �s��+1�� �C��
//	//// �s���_�ƴN�����p���C��
//	//// �C�@�� Slices �̫��� vertex ��}�Y�e��� vertex ���|�A�ҥH�ϥθӨ�� vertex ���C��
//	//for (int i = 1; i < _iStacks; i++ ) {  
//	//	for( int j = 0 ; j < 2*(_iSlices+1) - 2 ; j++ ) {
//	//		if( j%2 ) _pColors[i*2*(_iSlices+1)+j] = PhongReflectionModel(_pPoints[i*2*(_iSlices+1)+j], _pNormals[i*2*(_iSlices+1)+j], vLightPos, vLightI);
//	//		else _pColors[i*2*(_iSlices+1)+j] =  _pColors[(i-1)*2*(_iSlices+1)+j+1];		
//	//	}
//	//	_pColors[(i+1)*2*(_iSlices+1)-2] = _pColors[i*2*(_iSlices+1)];
//	//	_pColors[(i+1)*2*(_iSlices+1)-1] = _pColors[i*2*(_iSlices+1)+1];
//	//}
//
//	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
//}