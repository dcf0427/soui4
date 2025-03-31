﻿#pragma once

using namespace SOUI;

#include "matrix/SMatrix.h"

namespace SOUI{

class SClock : public SImageWnd
{
	DEF_SOBJECT(SImageWnd, L"clock")

public:
	SClock();

	enum {TIMER_REFRESH = 1};

protected:
    SMatrix InitMatrix(double angle, CPoint &center);

    double GetMinuteSecondAngle(int nValue);
    double GetHourAngle(int nHour,int nMinute);

protected:
	int  OnCreate(void*);
	void OnPaint(SOUI::IRenderTarget * pRT);
	void OnTimer(char cTimerID);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_TIMER_EX(OnTimer)
		MSG_WM_CREATE(OnCreate)
	SOUI_MSG_MAP_END()

protected:
	SOUI_ATTRS_BEGIN()
		ATTR_IMAGEAUTOREF(L"pointer_hour",   pointer_hour, TRUE)
		ATTR_IMAGEAUTOREF(L"pointer_minute", pointer_minute, TRUE)
		ATTR_IMAGEAUTOREF(L"pointer_second", pointer_second, TRUE)
	SOUI_ATTRS_END()

	SAutoRefPtr<IBitmapS> pointer_hour;
	SAutoRefPtr<IBitmapS> pointer_minute;
	SAutoRefPtr<IBitmapS> pointer_second;
};

}