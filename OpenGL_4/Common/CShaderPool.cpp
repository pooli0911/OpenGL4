#include <cstring>
#include "CShaderPool.h"

CShaderPool* CShaderPool::_pShaderPool = nullptr;

void CShaderPool::destroyInstance()
{
	CC_SAFE_DELETE(_pShaderPool);
}


CShaderPool::CShaderPool() {
	_iNumShaders = 0;
	for (int i = 0; i < SHADERPOOL_MAX; i++) {
		_Pool[i].vsName[0] = '\0';
		_Pool[i].fsName[0] = '\0';
		_Pool[i].uiShaderID = MAX_UNSIGNED_INT;
	}
}

CShaderPool::~CShaderPool()
{

}

CShaderPool* CShaderPool::create()
{
	if (_pShaderPool == nullptr) {
		_pShaderPool = new (std::nothrow) CShaderPool;
	}
	return _pShaderPool;
}

CShaderPool* CShaderPool::getInstance()
{
	if (_pShaderPool == nullptr){
		_pShaderPool = new (std::nothrow) CShaderPool;
	}
	return _pShaderPool;
}

GLuint CShaderPool::addShader(char* vsName, char* fsName)
{
	// 先將 vsName 與 fsName 複製到 vtxShaderName與 pxlShaderName 中
	strcpy_s(_Pool[_iNumShaders].vsName, strlen(vsName) + 1, vsName);
	strcpy_s(_Pool[_iNumShaders].fsName, strlen(fsName) + 1, fsName);
	_Pool[_iNumShaders].uiShaderID = InitShader(vsName, fsName);
	_iNumShaders++;
	return(_Pool[_iNumShaders-1].uiShaderID);
}

GLuint CShaderPool::getShaderID(char* vsName, char* fsName)
{
	int i;
	for ( i = 0; i < _iNumShaders ; i++)
	{
		if(strcmp(vsName, _Pool[i].vsName) == 0 && strcmp(fsName, _Pool[i].fsName) == 0)  return(_Pool[i].uiShaderID);
	}
	return(addShader(vsName, fsName));

//	return(MAX_UNSIGNED_INT);
	
}