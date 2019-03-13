//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

class DisassemblyWindow
{
public:
	DisassemblyWindow();

private:
	void MakeFont();
	void PrintText(HDC hdc, int x, int y, char *pch, int len);
	void OnPaint();
	//-------------------------------------------------------------------------------------------------
	const int LineSpaceAbove = 1;
	const int LineSpaceBelow = 1;
	const int LineAdditionalSpace = LineSpaceAbove + LineSpaceBelow;

	HWND m_hwnd;
	HFONT m_hFont;
	int m_iFontSize;
	int m_iFontZoom;
	int m_cxChar;
	int m_cyChar;
};

//-------------------------------------------------------------------------------------------------

DisassemblyWindow::DisassemblyWindow()
{
	m_iFontSize = 10;
	m_iFontZoom = 0;
}

//-------------------------------------------------------------------------------------------------

void DisassemblyWindow::MakeFont()
{
	if ( m_hFont )
	{
		DeleteObject( m_hFont );
	}

	HDC hdc = GetDC( m_hwnd );
	int nHeight = -MulDiv( m_iFontSize + m_iFontZoom, GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
	ReleaseDC( m_hwnd, hdc );

	int cset = ANSI_CHARSET;
	static const TCHAR facename[ ] = _T( "Lucida Console" );

	m_hFont = CreateFont( nHeight, 0, 0, 0, 0, 0, 0, 0, cset, OUT_DEFAULT_PRECIS,
						  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, facename );

	HFONT of = ( HFONT )SelectObject(hdc, m_hFont);
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	m_cxChar = tm.tmAveCharWidth;
	// Reserve 2 pixels for things like bookmark indicators
	m_cyChar = tm.tmHeight + tm.tmExternalLeading + LineAdditionalSpace;

	SelectObject(hdc, of);
	ReleaseDC (m_hwnd, hdc);
}

//-------------------------------------------------------------------------------------------------

void DisassemblyWindow::PrintText(HDC hdc, int x, int y, char *pch, int len)
{
	const RECT rc = { x * m_cxChar, y * m_cyChar, rc.left + len * m_cxChar, rc.top + m_cyChar + LineAdditionalSpace };
	ExtTextOutA(hdc, rc.left, rc.top + LineSpaceAbove, ETO_OPAQUE, &rc, pch, len, 0);
}

/*
case WM_VSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof (si);
        si.fMask  = SIF_ALL;
        GetScrollInfo (hwnd, SB_VERT, &si);

        // Save the position for comparison later on.
        yPos = si.nPos;
        switch (LOWORD (wParam))
        {

        // User clicked the HOME keyboard key.
        case SB_TOP:
            si.nPos = si.nMin;
            break;
              
        // User clicked the END keyboard key.
        case SB_BOTTOM:
            si.nPos = si.nMax;
            break;
              
        // User clicked the top arrow.
        case SB_LINEUP:
            si.nPos -= 1;
            break;
              
        // User clicked the bottom arrow.
        case SB_LINEDOWN:
            si.nPos += 1;
            break;
              
        // User clicked the scroll bar shaft above the scroll box.
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
              
        // User clicked the scroll bar shaft below the scroll box.
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
              
        // User dragged the scroll box.
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
              
        default:
            break; 
        }

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo (hwnd, SB_VERT, &si);

        // If the position has changed, scroll window and update it.
        if (si.nPos != yPos)
        {                    
            ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
            UpdateWindow (hwnd);
        }

        return 0;
		*/
//-------------------------------------------------------------------------------------------------

void DisassemblyWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint( m_hwnd, &ps );

	//-------------------------------------------------------
	HideCaret( m_hwnd );
	/*
	// Delete remains of last position.
	int a = iVscrollPos + ps.rcPaint.top / cyChar;
	int b = iVscrollPos + ps.rcPaint.bottom / cyChar;
	if (b >= iNumlines)
		b = iNumlines - 1;
	int iBkColor = PALETTERGB(GetRValue(iBkColorValue), GetGValue(iBkColorValue), GetBValue(iBkColorValue));
	RECT rc;
	HBRUSH hbr = CreateSolidBrush(iBkColor);
	// Delete lower border if there are empty lines on screen.
	GetClientRect(hwnd, &rc);
	rc.top = (b - iVscrollPos + 1) * cyChar;
	if (rc.top < rc.bottom)
		FillRect(hdc, &rc, hbr);
	// Delete right border.
	GetClientRect(hwnd, &rc);
	rc.left = (iHscrollMax + 1 - iHscrollPos) * cxChar;
	if (rc.left < rc.right)
		FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	*/

	int iTextColorValue = GetSysColor( COLOR_WINDOWTEXT );
	int iBkColorValue = GetSysColor( COLOR_WINDOW );
	int iSelTextColorValue = GetSysColor( COLOR_HIGHLIGHTTEXT );
	int	iSelBkColorValue = GetSysColor( COLOR_HIGHLIGHT );
	int	iSepColorValue = RGB( 192, 192, 192 );
	int iBmkColor = RGB( 255, 0, 0 );

	int iBkColor = PALETTERGB( GetRValue( iBkColorValue ), GetGValue( iBkColorValue ), GetBValue( iBkColorValue ) );
	int iTextColor = PALETTERGB( GetRValue( iTextColorValue ), GetGValue( iTextColorValue ), GetBValue( iTextColorValue ) );
	int iSelBkColor = PALETTERGB( GetRValue( iSelBkColorValue ), GetGValue( iSelBkColorValue ), GetBValue( iSelBkColorValue ) );
	int iSelTextColor = PALETTERGB( GetRValue( iSelTextColorValue ), GetGValue( iSelTextColorValue ), GetBValue( iSelTextColorValue ) );

	// Get font.
	HGDIOBJ oldfont = SelectObject( hdc, m_hFont );
	HPEN sep_pen = CreatePen( PS_SOLID, 1, iSepColorValue );
	HGDIOBJ oldpen = SelectObject( hdc, sep_pen );
	HBRUSH hbr = CreateSolidBrush( iBmkColor );

	// do text here
	/*
	for (int i = a ; i <= b ; i++)
	{
		print_line(hdc, i, hbr);
		// Mark character.
		if (i == iCurByte / iBytesPerLine && !bSelected && GetFocus() == hwnd)
			mark_char(hdc);
	}
	*/

	SetTextColor( hdc, iTextColor );
	SetBkColor( hdc, iBkColor );

	DeleteObject( hbr );
	SelectObject( hdc, oldpen );
	DeleteObject( sep_pen );
	SelectObject( hdc, oldfont );

	ShowCaret( m_hwnd );
	EndPaint( m_hwnd, &ps );
}

//-------------------------------------------------------------------------------------------------

