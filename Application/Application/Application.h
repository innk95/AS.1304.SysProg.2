
// Application.h : главный файл заголовка для приложения PROJECT_NAME
//
#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы
#include "Game.h"

// CApplicationApp:
// О реализации данного класса см. Application.cpp
//


class CApplicationApp : public CWinApp
{
	

public:
	Game m_Game;
	CApplicationApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CApplicationApp theApp;