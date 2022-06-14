// Gouraud shading with a single light source at (4, 4, 0);
// Part 1 : Per vertex lighting is computed by CPU
// Part 2 : The control of the light source (position and intensity (RGB) )
//			Light's position could be moved automatically with Key A/a 
//			Light's intensity could be changed with Key R/r G/g B/b
// Part 3 : Geometry objects' materials
//

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common/CLineSegment.h"

#include "Common/CCamera.h"
#include "Common/CShaderPool.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

#define SETTING_MATERIALS 

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CChecker* g_pChecker;
CSolidCube* g_pCube;
CSolidSphere* g_pSphere;

// For View Point
GLfloat g_fRadius = 10.0;
GLfloat g_fTheta = 60.0f * DegreesToRadians;
GLfloat g_fPhi = 45.0f * DegreesToRadians;

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 4;
float g_fLightTheta = 0;

float g_fLightR = 0.95f;
float g_fLightG = 0.95f;
float g_fLightB = 0.95f;
CWireSphere* g_pLight;
vec4 g_vLight(5.0f, 5.0f, 0.0f, 1.0f); // x = r cos(theta) = 4, z = r sin(theta) = 0

LightSource g_Light1 = {
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	point4(5.0f, 5.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),		  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),		  // spotDirection�A�ݭ��s�p��
	1.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	60.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff = cos(spotCutoff) ; spot ���ө��d��� cos
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0,	// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
	LightType::OMNI_LIGHT // ���B�]�w�� SPOT_LIGHT �N��ݨ� Spot Light ���ĪG
};

// for Spot Light use only
CLineSegment* g_LightLine;
bool g_bUpdateLight = false;
//----------------------------------------------------------------------------

//  Part 3 ���誺�������-----------------------------------------------------
bool g_bColorOn = false;

//----------------------------------------------------------------------------
// �禡���쫬�ŧi
extern void IdleProcess();
void releaseResources();

void init(void)
{
	mat4 mxT;
	vec4 vT, vColor;
	// ���ͩһݤ� Model View �P Projection Matrix

	point4  eye(g_fRadius * sin(g_fTheta) * sin(g_fPhi), g_fRadius * cos(g_fTheta), g_fRadius * sin(g_fTheta) * cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// ���ͪ��󪺹���
	g_pChecker = new CChecker(GRID_SIZE);
	g_pChecker->setShadingMode(ShadingMode::GOURAUD_SHADING_CPU);
	g_pChecker->setShader();

	// �]�w Cube
	g_pCube = new CSolidCube;
	vT.x = 1.5; vT.y = 0.5; vT.z = -1.5;
	mxT = Translate(vT);
	g_pCube->setTRSMatrix(mxT);
	g_pCube->setShadingMode(ShadingMode::GOURAUD_SHADING_CPU);
	g_pCube->setShader();

	// �]�w Sphere
	g_pSphere = new CSolidSphere(1, 16, 16);
	vT.x = -1.5; vT.y = 1.0; vT.z = 1.5;
	mxT = Translate(vT);
	g_pSphere->setTRSMatrix(mxT);
	g_pSphere->setShadingMode(ShadingMode::GOURAUD_SHADING_CPU);
	g_pSphere->setShader();


	// �]�w �N�� Light �� WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	mxT = Translate(g_vLight);
	g_pLight->setTRSMatrix(mxT);
	g_pLight->setColor(g_Light1.diffuse);
	g_pLight->setShader();

	if (g_Light1.type == LightType::SPOT_LIGHT) {
		// �p�� SpotDirection Vector �P�ɥ��W�Ʀ����V�q
		g_Light1.UpdateDirection();
		g_LightLine = new CLineSegment(g_Light1.position, g_Light1.spotTarget, vec4(1, 0, 0, 1));
		g_LightLine->setShader();
	}

	// �]�����d�Ҥ��|�ʨ� Projection Matrix �ҥH�b�o�̳]�w�@���Y�i
	// �N���g�b OnFrameMove ���C���� Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->setProjectionMatrix(mpx);
	g_pCube->setProjectionMatrix(mpx);
	g_pSphere->setProjectionMatrix(mpx);
	g_pLight->setProjectionMatrix(mpx);
	if (g_Light1.type == LightType::SPOT_LIGHT) g_LightLine->setProjectionMatrix(mpx);

}

//----------------------------------------------------------------------------
void GL_Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window

	g_pChecker->draw();
	g_pSphere->draw();
	g_pCube->draw();
	g_pLight->draw();
	if (g_Light1.type == LightType::SPOT_LIGHT) g_LightLine->draw();
	glutSwapBuffers();	// �洫 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void UpdateLightPosition(float dt)
{
	mat4 mxT;
	// �C��¶ Y �b�� 90 ��
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime * (float)M_PI_2;
	if (g_fLightTheta >= (float)M_PI * 2.0f) {
		g_fLightTheta -= (float)M_PI * 2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_vLight.x = g_fLightRadius * cosf(g_fLightTheta);
	g_vLight.z = g_fLightRadius * sinf(g_fLightTheta);
	mxT = Translate(g_vLight);
	g_pLight->setTRSMatrix(mxT);
	g_Light1.position.x = g_vLight.x;
	g_Light1.position.z = g_vLight.z;

	if (g_Light1.type == LightType::SPOT_LIGHT) {
		g_LightLine->updatePosition(g_Light1.position, g_Light1.spotTarget);
		g_bUpdateLight = true;
	}
}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{
	// Part1 : ���s�p�� Light ����m
	if (g_bAutoRotating) UpdateLightPosition(delta);

	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view �P projection matrix �O�_�ݭn��s������
	auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pChecker->setViewMatrix(mvx);
		g_pCube->setViewMatrix(mvx);
		g_pSphere->setViewMatrix(mvx);
		g_pLight->setViewMatrix(mvx);
		if (g_Light1.type == LightType::SPOT_LIGHT) g_LightLine->setViewMatrix(mvx);
	}

	if ( g_bUpdateLight ) {
		g_Light1.spotDirection.x = g_Light1.spotTarget.x - g_Light1.position.x;
		g_Light1.spotDirection.y = g_Light1.spotTarget.y - g_Light1.position.y;
		g_Light1.spotDirection.z = g_Light1.spotTarget.z - g_Light1.position.z;
		g_Light1.spotDirection = normalize(g_Light1.spotDirection);
		g_LightLine->updatePosition(g_Light1.position, g_Light1.spotTarget);
		g_bUpdateLight = false;
	}

	// �p�G�ݭn���s�p��ɡA�b�o��p��C�@�Ӫ����C��
	g_pChecker->update(delta, g_Light1);
	g_pCube->update(delta, g_Light1);
	g_pSphere->update(delta, g_Light1);
	g_pLight->update(delta); // �N����������

	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case  SPACE_KEY:  // Part 3 : materials
		if (g_bColorOn) { // �ثe���C��A�������Ƕ�
			g_pChecker->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pChecker->setKaKdKsShini(0, 0.8f, 0.2f, 1);
			g_pCube->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pCube->setKaKdKsShini(0, 0.8f, 0.2f, 1);
			g_pSphere->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pSphere->setKaKdKsShini(0, 0.8f, 0.2f, 1);
		}
		else { // �ثe���Ƕ��A���������C��
			g_pChecker->setMaterials(vec4(0), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pChecker->setKaKdKsShini(0, 0.8f, 0.5f, 1);
			g_pCube->setMaterials(vec4(0), vec4(0.85f, 0, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pCube->setKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
			g_pSphere->setMaterials(vec4(0), vec4(0, 0, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pSphere->setKaKdKsShini(0.15f, 0.53f, 0.78f, 4);
		}
		g_bColorOn = !g_bColorOn;
		break;
		//----------------------------------------------------------------------------
		// Part 2 : for single light source
	case 65: // A key
	case 97: // a key
		g_bAutoRotating = !g_bAutoRotating;
		break;
	case 82: // R key
		if (g_fLightR <= 0.95f) g_fLightR += 0.05f;
		g_Light1.ambient.x = g_fLightR;  g_Light1.diffuse.x = g_fLightR; g_Light1.specular.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 114: // r key
		if (g_fLightR >= 0.05f) g_fLightR -= 0.05f;
		g_Light1.ambient.x = g_fLightR;  g_Light1.diffuse.x = g_fLightR; g_Light1.specular.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 71: // G key
		if (g_fLightG <= 0.95f) g_fLightG += 0.05f;
		g_Light1.ambient.y = g_fLightG;  g_Light1.diffuse.y = g_fLightG; g_Light1.specular.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 103: // g key
		if (g_fLightG >= 0.05f) g_fLightG -= 0.05f;
		g_Light1.ambient.y = g_fLightG;  g_Light1.diffuse.y = g_fLightG; g_Light1.specular.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 66: // B key
		if (g_fLightB <= 0.95f) g_fLightB += 0.05f;
		g_Light1.ambient.z = g_fLightB;  g_Light1.diffuse.z = g_fLightB; g_Light1.specular.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 98: // b key
		if (g_fLightB >= 0.05f) g_fLightB -= 0.05f;
		g_Light1.ambient.z = g_fLightB;  g_Light1.diffuse.z = g_fLightB; g_Light1.specular.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
		//----------------------------------------------------------------------------
	case 033:
		glutIdleFunc(NULL);
		releaseResources();
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:   // �ثe���U���O�ƹ�����
		//if ( state == GLUT_DOWN ) ; 
		break;
	case GLUT_MIDDLE_BUTTON:  // �ثe���U���O�ƹ����� �A���� Y �b
		//if ( state == GLUT_DOWN ) ; 
		break;
	case GLUT_RIGHT_BUTTON:   // �ثe���U���O�ƹ��k��
		//if ( state == GLUT_DOWN ) ;
		break;
	default:
		break;
	}
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {
	// �q�o�̱��� SpotLight target ����m
	// ���k������ target �� X �b�y�СA�W�U�������� target �� Z �b�y��
	switch (key) {
	case GLUT_KEY_LEFT:		// �ثe���U���O�V����V��
		g_Light1.spotTarget.x -= 0.1f;
		g_bUpdateLight = true;
		break;
	case GLUT_KEY_RIGHT:	// �ثe���U���O�V�k��V��
		g_Light1.spotTarget.x += 0.1f;
		g_bUpdateLight = true;
		break;
	case GLUT_KEY_UP:	// �ثe���U���O�V�k��V��
		g_Light1.spotTarget.z -= 0.1f;
		g_bUpdateLight = true;
		break;
	case GLUT_KEY_DOWN:	// �ثe���U���O�V�k��V��
		g_Light1.spotTarget.z += 0.1f;
		g_bUpdateLight = true;
		break;
	default:
		break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {

	g_fPhi = (float)-M_PI * (x - HALF_SIZE) / (HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI * (float)y / SCREEN_SIZE;
	point4  eye(g_fRadius * sin(g_fTheta) * sin(g_fPhi), g_fRadius * cos(g_fTheta), g_fRadius * sin(g_fTheta) * cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)-M_PI * (x - HALF_SIZE) / (HALF_SIZE);  // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI * (float)y / SCREEN_SIZE;

	point4  eye(g_fRadius * sin(g_fTheta) * sin(g_fPhi), g_fRadius * cos(g_fTheta), g_fRadius * sin(g_fTheta) * cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);

	//  ���� projection �x�}�A���B�����ͥ���v�x�}
	g_mxProjection = Perspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);

	g_pCube->setProjectionMatrix(g_mxProjection);
	g_pSphere->setProjectionMatrix(g_mxProjection);
	// �]�w�ѽL�� Projection Matrix
	g_pChecker->setProjectionMatrix(g_mxProjection);
	g_pLight->setProjectionMatrix(g_mxProjection);

	glClearColor(0.0, 0.0, 0.0, 1.0); // black background
	glEnable(GL_DEPTH_TEST);
}
//----------------------------------------------------------------------------

void releaseResources()
{
	delete g_pCube;
	delete g_pSphere;
	delete g_pChecker;
	delete g_pLight;
	if (g_Light1.type == LightType::SPOT_LIGHT) delete g_LightLine;
	CCamera::getInstance()->destroyInstance();
	CShaderPool::getInstance()->destroyInstance();
}


int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE, SCREEN_SIZE);

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("Shading Example 2");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE;
	glewInit();

	init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);
	glutKeyboardFunc(Win_Keyboard);	// �B�z ASCI ����p A�Ba�BESC ��...����
	glutSpecialFunc(Win_SpecialKeyboard);	// �B�z NON-ASCI ����p F1�BHome�B��V��...����
	glutDisplayFunc(GL_Display);
	glutReshapeFunc(GL_Reshape);
	glutIdleFunc(IdleProcess);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	// �o�@��|�������Q�j�������ɡA�{��������|�^�� glutMainLoop(); ���U�@��
	glutMainLoop();
	releaseResources();
	return 0;
}