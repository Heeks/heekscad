// HeeksPrintout.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksPrintout.h"
#include "wx/print.h"
#include "HeeksFrame.h"


bool HeeksPrintout::OnPrintPage(int page)
{
	wxGetApp().m_frame->m_printout = this;

    wxDC *dc = GetDC();
    if (dc)
    {
		SetUnitsFactor();

		wxGetApp().m_frame->Draw(*GetDC());

        // Draw page numbers at top left corner of printable area, sized so that
        // screen size of text matches paper size.
#if 0
        MapScreenSizeToPage();
        wxChar buf[200];
        wxSprintf(buf, wxT("PAGE %d"), page);
        dc->DrawText(buf, 0, 0);
#endif
        return true;
    }
    else
        return false;
}

bool HeeksPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

void HeeksPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool HeeksPrintout::HasPage(int pageNum)
{
    return (pageNum == 1 || pageNum == 2);
}

void HeeksPrintout::SetUnitsFactor()
{
    wxDC *dc = GetDC();

    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);

    // This scales the DC so that the printout roughly represents the the screen
    // scaling. The text point size _should_ be the right size but in fact is
    // too small for some reason. This is a detail that will need to be
    // addressed at some point but can be fudged for the moment.
    float scalex = (float)((float)ppiPrinterX/(float)ppiScreenX);

    // Now we have to check in case our real page size is reduced (e.g. because
    // we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    int w, h;
    dc->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);

    // If printer pageWidth == current DC width, then this doesn't change. But w
    // might be the preview bitmap width, so scale down.
    float overallScalex = scalex * (float)(w/(float)pageWidth);

    // Calculate conversion factor for converting millimetres into logical
    // units. There are approx. 25.4 mm to the inch. There are ppi device units
    // to the inch. Therefore 1 mm corresponds to ppi/25.4 device units. We also
    // divide by the screen-to-printer scaling factor, because we need to
    // unscale to pass logical units to DrawLine.
    m_logUnitsFactorx = (float)(ppiPrinterX/25.4) * (float)(w/(float)pageWidth);
    m_logUnitsFactory = (float)(ppiPrinterY/25.4) * (float)(h/(float)pageHeight);

    m_scale = overallScalex;

    wxRect fitRect = GetLogicalPageMarginsRect(*wxGetApp().m_pageSetupData);
    m_xoff = fitRect.x + fitRect.width / 2;
    m_yoff = fitRect.y + fitRect.height / 2;
}

void HeeksPrintout::SetColor(const HeeksColor &c)
{
	HeeksColor col = c;
	if(c == HeeksColor(255, 255, 255))col = HeeksColor(0, 0, 0);
	wxPen pen(col.COLORREF_color(), (int)(m_scale + 0.99));
    wxDC *dc = GetDC();
    dc->SetPen(pen);
}

void HeeksPrintout::DrawLine(const double* s, const double* e)
{
    wxDC *dc = GetDC();
	dc->DrawLine(m_xoff + (long)(s[0] * m_logUnitsFactorx + 0.5), m_yoff - (long)(s[1] * m_logUnitsFactory + 0.5), m_xoff + (long)(e[0] * m_logUnitsFactorx + 0.5), m_yoff - (long)(e[1] * m_logUnitsFactory + 0.5));
}

void HeeksPrintout::DrawArc(const double* s, const double* e, const double* c)
{
    wxDC *dc = GetDC();

	int isx = m_xoff + (long)(e[0] * m_logUnitsFactorx + 0.5);
	int isy = m_yoff - (long)(e[1] * m_logUnitsFactory + 0.5);
	int iex = m_xoff + (long)(s[0] * m_logUnitsFactorx + 0.5);
	int iey = m_yoff - (long)(s[1] * m_logUnitsFactory + 0.5);
	int icx = m_xoff + (long)(c[0] * m_logUnitsFactorx + 0.5);
	int icy = m_yoff - (long)(c[1] * m_logUnitsFactory + 0.5);

    dc->SetBackgroundMode(wxTRANSPARENT);
    dc->SetBrush(*wxTRANSPARENT_BRUSH);
	dc->DrawArc(isx, isy, iex, iey, icx, icy);
}
