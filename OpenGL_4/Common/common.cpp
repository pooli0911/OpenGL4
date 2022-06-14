#include <iostream>
#include <cmath>
#include <ctime>
#include "Timer.h"
#include <string.h>
#include "..\GL\glut.h"


int g_ifps;
int g_icount = 0;
double g_ckStart = -1;
double g_felapse;
double g_fprev;

Timer g_Timer;

extern void onFrameMove(float delta);


void IdleProcess()
{
	float delta;
	double ckNow; 
	g_icount++;
	if( g_ckStart == -1 ) {
		g_fprev = g_ckStart = g_Timer.getElapsedTimeInMilliSec(); //clock(); //開始計時
		onFrameMove(0);
	}
	else {
		ckNow = g_Timer.getElapsedTimeInMilliSec();
		g_felapse = (ckNow - g_ckStart);
		if( g_felapse >= 1000.0 ) {
			g_ifps = g_icount;
			g_icount = 0;
			g_ckStart += g_felapse;
		}
		delta = (float)((ckNow - g_fprev)/1000.0); // 計算間隔的時間
		g_fprev = ckNow; // 記錄這次的時間，當成前一次的時間
		onFrameMove(delta);
		//printf("%d\n",g_ifps);
	}
	glutPostRedisplay(); // 呼叫 Rendering 更新螢幕
}
