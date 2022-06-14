#include "CLineSegment.h"
#include "CShaderPool.h"
// Example 3 開始改成面朝上(Y軸)


CLineSegment::CLineSegment(const vec4 SPoint, const vec4 EPoint, const vec4 vColor)
{
	// 預設在 X 軸上的 (-1,0,0) 到 (1,0,0) 的線段
	_Points[0] = SPoint; _Points[1] = EPoint;

	// 預設為紅色
	_Colors[0] = _Colors[1] = vColor;  // (r, g, b, a)

	// Create and initialize a buffer object 
	createBufferObject();

	_bUpdateView = false;
	_bUpdateProj = false;
}


void CLineSegment::createBufferObject()
{
    glGenVertexArrays( 1, &_uiVao );
    glBindVertexArray( _uiVao );

    // Create and initialize a buffer object

    glGenBuffers( 1, &_uiBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(_Points) + sizeof(_Colors), NULL, GL_STATIC_DRAW );

    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(_Points), _Points ); 
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(_Points), sizeof(_Colors), _Colors );
}

void CLineSegment::setShader(mat4 &mxModelView, mat4 &mxProjection, GLuint uiShaderHandle)
{
    // Load shaders and use the resulting shader program
	if (uiShaderHandle == MAX_UNSIGNED_INT) {
		_uiProgram = CShaderPool::getInstance()->getShaderID("vsVtxColor.glsl", "fsVtxColor.glsl");
	}
	else _uiProgram = uiShaderHandle;

	glUseProgram(_uiProgram);
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( _uiProgram, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( _uiProgram, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(_Points)) );

	_uiModelView = glGetUniformLocation( _uiProgram, "ModelView" );
	_mxMVFinal = _mxView = mxModelView;
	glUniformMatrix4fv( _uiModelView, 1, GL_TRUE, _mxView );

    _uiProjection = glGetUniformLocation( _uiProgram, "Projection" );
	_mxProjection = mxProjection;
	glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void CLineSegment::setShader(GLuint uiShaderHandle)
{
	// Load shaders and use the resulting shader program
	if (uiShaderHandle == MAX_UNSIGNED_INT) {
		_uiProgram = CShaderPool::getInstance()->getShaderID("vsVtxColor.glsl", "fsVtxColor.glsl");
	}
	else _uiProgram = uiShaderHandle;

	glUseProgram(_uiProgram);
	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(_uiProgram, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(_uiProgram, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(_Points)));

	_uiModelView = glGetUniformLocation(_uiProgram, "ModelView");
	// _mxMVFinal, _mxView 宣告時就是單位矩陣
	glUniformMatrix4fv(_uiModelView, 1, GL_TRUE, _mxView);

	_uiProjection = glGetUniformLocation(_uiProgram, "Projection");
	// _mxProjection 宣告時就是單位矩陣
	glUniformMatrix4fv(_uiProjection, 1, GL_TRUE, _mxProjection);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CLineSegment::updatePosition(vec4 SPoint, vec4 EPoint)
{
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	_Points[0] = SPoint; _Points[1] = EPoint;
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_Points), _Points);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CLineSegment::setViewMatrix(mat4 &mat)
{
	_mxView = mat;
	_bUpdateView = true;
}

void CLineSegment::setProjectionMatrix(mat4 &mat)
{
	_mxProjection = mat;
	_bUpdateProj = true;
}

void CLineSegment::setTRSMatrix(mat4 &mat)
{
	_mxTRS = mat;
	_bUpdateView = true;
}

void CLineSegment::setColor(vec4 vColor)
{
	_Colors[0] = _Colors[1] = vColor;
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(_Points), sizeof(_Colors), _Colors );
}

void CLineSegment::setVtxColors(vec4 SPColor, vec4 EPColor)
{
	_Colors[0] = SPColor;
	_Colors[1] = EPColor;
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(_Points), sizeof(_Colors), _Colors );
}

void CLineSegment::draw()
{
	glUseProgram( _uiProgram );
	glBindVertexArray( _uiVao );
	if( _bUpdateView ) {
		_mxMVFinal = _mxView * _mxTRS;
		_bUpdateView = false;
	}
	glUniformMatrix4fv(_uiModelView, 1, GL_TRUE, _mxMVFinal);

	if( _bUpdateProj ) {
		glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
		_bUpdateProj = false;
	}
	glDrawArrays( GL_LINES, 0, VERTEX_NUM );
}

void CLineSegment::drawW()
{
	glBindVertexArray( _uiVao );
	if( _bUpdateView ) {
		_mxMVFinal = _mxView * _mxTRS;
		_bUpdateView = false;
	}
	glUniformMatrix4fv(_uiModelView, 1, GL_TRUE, _mxMVFinal);

	if( _bUpdateProj ) {
		glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
		_bUpdateProj = false;
	}

	glDrawArrays( GL_LINES, 0, VERTEX_NUM );
}
