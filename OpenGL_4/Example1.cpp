// Flat shading with a single light source at (3, 4, 3) 
//

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CWireSphere.h"
#include "Common/CChecker.h"
#include "Common/CCamera.h"
#include "Common/CShaderPool.h"

#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

//----------------------------------------------------------------------------
// �禡���쫬�ŧi
extern void IdleProcess();
void releaseResources();

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CChecker *g_pChecker;
CSolidCube *g_pCube1, *g_pCube2;

// For View Point
GLfloat g_fRadius = 10.0;
GLfloat g_fTheta = 60.0f * DegreesToRadians;
GLfloat g_fPhi = 45.0f * DegreesToRadians;

//----------------------------------------------------------------------------
// for single light source
CWireSphere *g_pLight;
vec4 g_LightPos( 3, 4, 3, 1);
vec4 g_LightInt( 0.9f, 0.95f, 0.9f, 1); 

void init( void )
{
	mat4 mxT;
	vec4 vT, vColor;
	// �إ����Y
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);

	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	// ���ͦa�O����
	g_pChecker = new CChecker(GRID_SIZE);
	g_pChecker->setShader();

	// ���ͥߤ��骫��1
	g_pCube1 = new CSolidCube;
	g_pCube1->setShader();
	// �]�w Cube
	vT.x = 1.5; vT.y = 0.5; vT.z = -1.5;
	mxT = Translate(vT);
	g_pCube1->setTRSMatrix(mxT);

	// ���ͥߤ��骫��2
	g_pCube2 = new CSolidCube;
	g_pCube2->setShader();
	// �]�w Cube
	vT.x = -1.5; vT.y = 0.5; vT.z = 1.5;
	mxT = Translate(vT);
	g_pCube2->setTRSMatrix(mxT);

	// �]�w �N�� Light �� WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	g_pLight->setShader();
	mxT = Translate(g_LightPos);
	g_pLight->setTRSMatrix(mxT);
	g_pLight->setColor(g_LightInt);

	// �]�����d�Ҥ��|�ʨ� Projection Matrix �ҥH�b�o�̳]�w�@���Y�i
	// �N���g�b OnFrameMove ���C���� Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pChecker->setProjectionMatrix(mpx);
	g_pCube1->setProjectionMatrix(mpx);
	g_pCube2->setProjectionMatrix(mpx);
	g_pLight->setProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window
	g_pChecker->draw();
	g_pCube1->draw();
	g_pCube2->draw();
	g_pLight->draw();
	glutSwapBuffers();	// �洫 Frame Buffer
}

void onFrameMove(float delta)
{
	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view �P projection matrix �O�_�ݭn��s������
	auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pChecker->setViewMatrix(mvx);
		g_pCube1->setViewMatrix(mvx);
		g_pCube2->setViewMatrix(mvx);
		g_pLight->setViewMatrix(mvx);
	}

	g_pChecker->update(delta, g_LightPos, g_LightInt);
	g_pCube1->update(delta, g_LightPos, g_LightInt);
	g_pCube2->update(delta, g_LightPos, g_LightInt);
	g_pLight->update(delta);
	GL_Display();
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {

	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	CCamera::getInstance()->updateViewPosition(eye);
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE);  // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;

	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	CCamera::getInstance()->updateViewPosition(eye);
}

//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
}


//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
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

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case  SPACE_KEY:

		break;
	case 68: // D key
	case 100: // d key

		break;
    case 033:
		glutIdleFunc( NULL );
		releaseResources();
        exit( EXIT_SUCCESS );
        break;
    }
}

void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_LEFT:		// �ثe���U���O�V����V��

			break;
		case GLUT_KEY_RIGHT:	// �ثe���U���O�V�k��V��

			break;
		default:
			break;
	}
}

void releaseResources()
{
	CCamera::getInstance()->destroyInstance();
	CShaderPool::getInstance()->destroyInstance();
	delete g_pCube1;
	delete g_pCube2;
	delete g_pChecker;
	delete g_pLight;
}

//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Shading Exmaple 1");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// �B�z ASCI ����p A�Ba�BESC ��...����
	glutSpecialFunc( Win_SpecialKeyboard);	// �B�z NON-ASCI ����p F1�BHome�B��V��...����
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	// �o�@��|�������Q�j�������ɡA�{��������|�^�� glutMainLoop(); ���U�@��
    glutMainLoop();
	releaseResources();
    return 0;
}
