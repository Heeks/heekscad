// HeeksPrintout.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "wx/print.h"

class HeeksPrintout: public wxPrintout
{
public:
	float m_logUnitsFactorx;
	float m_logUnitsFactory;
    wxCoord m_xoff;
    wxCoord m_yoff;
	float m_scale;

	HeeksPrintout(const wxChar *title = _T("Heeks printout")):wxPrintout(title) {}
	bool OnPrintPage(int page);
	bool HasPage(int page);
	bool OnBeginDocument(int startPage, int endPage);
	void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
	void SetUnitsFactor();

	void SetColor(const HeeksColor &c);
	void DrawLine(const double* s, const double* e);
	void DrawArc(const double* s, const double* e, const double* c);
};

