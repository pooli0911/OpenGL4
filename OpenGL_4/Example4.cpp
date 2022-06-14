// Gouraud shading with a single light source at (4, 4, 0);
// 與 Exmaple4 相同, Light 的參數改成以結構的方式傳遞
// 同時實現 Per Pixel Lighting、Nonphotorealistic Shading  與 Silhouette
//

#include "header/Angel.h"
#include "Common/TypeDefine.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common/CCamera.h"
#include "Common/CShaderPool.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CChecker      *g_pChecker;
CSolidCube    *g_pCube;
CSolidSphere  *g_pSphere;

CQuad         *g_LeftWall, *g_RightWall;
CQuad         *g_FrontWall, *g_BackWall;

// For View Point
GLfloat g_fRadius = 10.0;
GLfloat g_fTheta = 60.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;

point4  g_vEye( g_fRadius*sin(g_fTheta)*cos(g_fPhi), g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta),  1.0 );
point4  g_vAt( 0.0, 0.0, 0.0, 1.0 );
vec4    g_vUp( 0.0, 1.0, 0.0, 0.0 );

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 6;
float g_fLightTheta = 0;

float g_fLightR = 0.95f;
float g_fLightG = 0.95f;
float g_fLightB = 0.95f;

LightSource g_Light1 = {
    color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	point4(g_fLightRadius, g_fLightRadius, 0.0f, 1.0f),   // position
    point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  //spotTarget
    vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	1.0f	,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot 的照明範圍
	1.0f	,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), 照明方向與被照明點之間的角度取 cos 後, cut off 的值
	1	,	// constantAttenuation	(a + bd + cd^2)^-1 中的 a, d 為光源到被照明點的距離
	0	,	// linearAttenuation	    (a + bd + cd^2)^-1 中的 b
	0	,	// quadraticAttenuation (a + bd + cd^2)^-1 中的 c
	LightType::OMNI_LIGHT
};

CWireSphere *g_pLight;
//----------------------------------------------------------------------------

//  Part 3 材質的切換顯示-----------------------------------------------------
bool g_bColorOn = false;

//----------------------------------------------------------------------------
// 函式的原型宣告
extern void IdleProcess();
void releaseResources();

void init( void )
{
	mat4 mxT;
	vec4 vT, vColor;
	// 產生所需之 Model View 與 Projection Matrix

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// 產生物件的實體
	g_pChecker = new CChecker(GRID_SIZE);
	g_pChecker->setShadingMode(ShadingMode::PHONG_SHADING);
	g_pChecker->setShader();

	vT.x = -10.0f; vT.y = 10.0f; vT.z = 0;
	mxT = Translate(vT);
	g_LeftWall = new CQuad;
	g_LeftWall->setColor(vec4(0.6f));
	g_LeftWall->setTRSMatrix(mxT*RotateZ(-90.0f)*Scale(20.0f, 1, 20.0f));
	g_LeftWall->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_LeftWall->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_LeftWall->setShadingMode(ShadingMode::PHONG_SHADING);
	g_LeftWall->setShader();

	vT.x = 10.0f; vT.y = 10.0f; vT.z = 0;
	mxT = Translate(vT);
	g_RightWall = new CQuad;
	g_RightWall->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_RightWall->setColor(vec4(0.6f));
	g_RightWall->setTRSMatrix(mxT*RotateZ(90.0f)*Scale(20.0f, 1, 20.0f));
	g_RightWall->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_RightWall->setShadingMode(ShadingMode::PHONG_SHADING);
	g_RightWall->setShader();

	vT.x = 0.0f; vT.y = 10.0f; vT.z = 10.0f;
	mxT = Translate(vT);
	g_FrontWall = new CQuad;
	g_FrontWall->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_FrontWall->setColor(vec4(0.6f));
	g_FrontWall->setTRSMatrix(mxT*RotateX(-90.0f)*Scale(20.0f, 1, 20.0f));
	g_FrontWall->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_FrontWall->setShadingMode(ShadingMode::PHONG_SHADING);
	g_FrontWall->setShader();

	vT.x = 0.0f; vT.y = 10.0f; vT.z = -10.0f;
	mxT = Translate(vT);
	g_BackWall = new CQuad;
	g_BackWall->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_BackWall->setColor(vec4(0.6f));
	g_BackWall->setTRSMatrix(mxT*RotateX(90.0f)*Scale(20.0f, 1, 20.0f));
	g_BackWall->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_BackWall->setShadingMode(ShadingMode::PHONG_SHADING);
	g_BackWall->setShader();

	// 設定 Cube
	g_pCube = new CSolidCube;
	vT.x = 1.5; vT.y = 0.5; vT.z = -1.5;
	mxT = Translate(vT);
	g_pCube->setTRSMatrix(mxT);
	g_pCube->setShadingMode(ShadingMode::PHONG_SHADING);
	g_pCube->setShader();

	// 設定 Sphere
	g_pSphere = new CSolidSphere(1, 16, 16);
	vT.x = -1.5; vT.y = 1.0; vT.z = 1.5;
	mxT = Translate(vT);
	g_pSphere->setTRSMatrix(mxT);
	g_pSphere->setShadingMode(ShadingMode::PHONG_SHADING);
	g_pSphere->setShader();

	// 設定 代表 Light 的 WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->setShader();
	mxT = Translate(g_Light1.position);
	g_pLight->setTRSMatrix(mxT);
	g_pLight->setColor(g_Light1.diffuse);
	g_pLight->setLightingDisable();

	// 因為本範例不會動到 Projection Matrix 所以在這裡設定一次即可
	// 就不寫在 OnFrameMove 中每次都 Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->setProjectionMatrix(mpx);
	g_pCube->setProjectionMatrix(mpx);
	g_pSphere->setProjectionMatrix(mpx);
	g_pLight->setProjectionMatrix(mpx);
	g_LeftWall->setProjectionMatrix(mpx);
	g_RightWall->setProjectionMatrix(mpx);
	g_FrontWall->setProjectionMatrix(mpx);
	g_BackWall->setProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	g_pChecker->draw();
	g_pSphere->draw();
	g_pCube->draw();
	g_pLight->draw();
	g_LeftWall->draw();
	g_RightWall->draw();
	g_FrontWall->draw();
	g_BackWall->draw();
	glutSwapBuffers();	// 交換 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void updateLightPosition(float dt)
{
	mat4 mxT;
	// 每秒繞 Y 軸轉 90 度
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime*(float)M_PI_2;
	if( g_fLightTheta >= (float)M_PI*2.0f ) {
		g_fLightTheta -= (float)M_PI*2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_Light1.position.x = g_fLightRadius * cosf(g_fLightTheta);
	g_Light1.position.z = g_fLightRadius * sinf(g_fLightTheta);
	mxT = Translate(g_Light1.position);
	g_pLight->setTRSMatrix(mxT);
}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{
	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view 與 projection matrix 是否需要更新給物件
	auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pChecker->setViewMatrix(mvx);
		g_pCube->setViewMatrix(mvx);
		g_pSphere->setViewMatrix(mvx);
		g_LeftWall->setViewMatrix(mvx);
		g_RightWall->setViewMatrix(mvx);
		g_FrontWall->setViewMatrix(mvx);
		g_BackWall->setViewMatrix(mvx);
		g_pLight->setViewMatrix(mvx);
	}

	if( g_bAutoRotating ) { // Part 2 : 重新計算 Light 的位置
		updateLightPosition(delta);
	}
	// 如果需要重新計算時，在這邊計算每一個物件的顏色
	g_pChecker->update(delta, g_Light1);
	g_pCube->update(delta, g_Light1);
	g_pSphere->update(delta, g_Light1);
	g_LeftWall->update(delta, g_Light1);
	g_RightWall->update(delta, g_Light1);
	g_FrontWall->update(delta, g_Light1);
	g_BackWall->update(delta, g_Light1);
	g_pLight->update(delta);

	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case  SPACE_KEY:
		if (g_bColorOn) { // 目前有顏色，切換成灰階
			g_pChecker->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pChecker->setKaKdKsShini(0, 0.8f, 0.2f, 1);
			g_pCube->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pCube->setKaKdKsShini(0, 0.8f, 0.2f, 1);
			g_pSphere->setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pSphere->setKaKdKsShini(0, 0.8f, 0.2f, 1);
		}
		else { // 目前為灰階，切換成有顏色
			g_pChecker->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0.85f, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pChecker->setKaKdKsShini(0, 0.8f, 0.5f, 1);
			g_pCube->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.85f, 0, 0, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pCube->setKaKdKsShini(0.15f, 0.8f, 0.2f, 2);
			g_pSphere->setMaterials(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0, 0, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
			g_pSphere->setKaKdKsShini(0.15f, 0.45f, 0.55f, 5);
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
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
//----------------------------------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		releaseResources();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // 目前按下的是滑鼠左鍵
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // 目前按下的是滑鼠中鍵 ，換成 Y 軸
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // 目前按下的是滑鼠右鍵
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_LEFT:		// 目前按下的是向左方向鍵

			break;
		case GLUT_KEY_RIGHT:	// 目前按下的是向右方向鍵

			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {

	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE); // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(eye, at);
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE);  // 轉換成 g_fPhi 介於 -PI 到 PI 之間 (-180 ~ 180 之間)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	auto camera = CCamera::getInstance();
	camera->updateViewLookAt(eye, at);
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------

void releaseResources()
{
	delete g_pCube;
	delete g_pSphere;
	delete g_pChecker;
	delete g_pLight;
	delete g_LeftWall;
	delete g_RightWall;
	delete g_FrontWall;
	delete g_BackWall;
	CCamera::getInstance()->destroyInstance();
	CShaderPool::getInstance()->destroyInstance();
}

int main( int argc, char **argv )
{  
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Shading Example 4");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// 處理 ASCI 按鍵如 A、a、ESC 鍵...等等
	glutSpecialFunc( Win_SpecialKeyboard);	// 處理 NON-ASCI 按鍵如 F1、Home、方向鍵...等等
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc(IdleProcess);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	// 這一行會讓視窗被強迫關閉時，程式的執行會回到 glutMainLoop(); 的下一行
	glutMainLoop();
	releaseResources();
    return 0;
}