﻿// Sscrollbar.h : implementation file
//    模块:滚动条控件
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "control/SScrollbar.h"

SNSBEGIN
/////////////////////////////////////////////////////////////////////////////
// SScrollBar
SScrollBar::SScrollBar()
    : m_pSkin(GETBUILTINSKIN(SKIN_SYS_SCROLLBAR))
    , m_uAllowSize((UINT)-1)
    , m_fadeFrames(0)
    , m_byThumbTrackMinAlpha(128)
    , m_sbHandler(this)
{
    memset(&m_si, 0, sizeof(SCROLLINFO));
    m_si.nTrackPos = -1;
    m_evtSet.addEvent(EVENTID(EventScroll));
}

SScrollBar::~SScrollBar()
{
}

BOOL SScrollBar::IsVertical() const
{
    return m_sbHandler.IsVertical();
}

int SScrollBar::SetPos(int nPos)
{
    if (nPos > (m_si.nMax - (int)m_si.nPage + 1))
        nPos = (m_si.nMax - m_si.nPage + 1);
    if (nPos < m_si.nMin)
        nPos = m_si.nMin;
    if (nPos != m_si.nPos)
    {
        if (m_si.nTrackPos == -1)
        {
            CRect rcOldThumb = m_sbHandler.GetPartRect(SB_THUMBTRACK);
            m_si.nTrackPos = nPos;
            CRect rcNewThumb = m_sbHandler.GetPartRect(SB_THUMBTRACK);
            CRect rcUnion;
            rcUnion.UnionRect(&rcOldThumb, &rcNewThumb);
            if (IsVisible())
            {
                IRenderTarget *pRT = GetRenderTarget(&rcUnion, GRT_PAINTBKGND);
                m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
                m_sbHandler.OnDraw(pRT, SB_THUMBTRACK);
                ReleaseRenderTarget(pRT);
            }
            m_si.nTrackPos = -1;
        }
        m_si.nPos = nPos;
    }
    return m_si.nPos;
}

int SScrollBar::GetPos()
{
    return m_si.nPos;
}

int SScrollBar::GetMax()
{
    return m_si.nMax;
}

int SScrollBar::GetMin()
{
    return m_si.nMin;
}

void SScrollBar::OnInitFinished(IXmlNode *pNode)
{
    __baseCls::OnInitFinished(pNode);
    SASSERT(m_pSkin);
    if (m_uAllowSize == -1)
    {
        m_uAllowSize = m_pSkin->GetSkinSize().cx / 9;
    }
}

void SScrollBar::OnPaint(IRenderTarget *pRT)
{
    if (!m_pSkin)
        return;
    m_sbHandler.OnDraw(pRT, SB_LINEUP);
    m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
    m_sbHandler.OnDraw(pRT, SB_THUMBTRACK);
    m_sbHandler.OnDraw(pRT, SB_LINEDOWN);
}

void SScrollBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_sbHandler.OnMouseUp(point);
    ReleaseCapture();
}

void SScrollBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();
    m_sbHandler.OnMouseDown(point);
}

void SScrollBar::OnMouseMove(UINT nFlags, CPoint point)
{
    m_sbHandler.OnMouseMove(point);
}

void SScrollBar::OnTimer(char nIDEvent)
{
    m_sbHandler.OnTimer(nIDEvent);
}

void SScrollBar::OnMouseHover(UINT nFlags, CPoint ptPos)
{
    m_sbHandler.OnMouseHover(ptPos);
}

void SScrollBar::OnMouseLeave()
{
    m_sbHandler.OnMouseLeave();
}

LRESULT SScrollBar::OnSetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bRedraw = wParam != 0;
    LPSCROLLINFO lpScrollInfo = (LPSCROLLINFO)lParam;
    if (lpScrollInfo->fMask & SIF_PAGE)
        m_si.nPage = lpScrollInfo->nPage;
    if (lpScrollInfo->fMask & SIF_POS)
        m_si.nPos = lpScrollInfo->nPos;
    if (lpScrollInfo->fMask & SIF_RANGE)
    {
        m_si.nMin = lpScrollInfo->nMin;
        m_si.nMax = lpScrollInfo->nMax;
    }
    if (m_si.nPos > (m_si.nMax - (int)m_si.nPage + 1))
        m_si.nPos = (m_si.nMax - m_si.nPage + 1);
    if (m_si.nPos < m_si.nMin)
        m_si.nPos = m_si.nMin;
    if (bRedraw)
        Invalidate();

    return TRUE;
}

LRESULT SScrollBar::OnGetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPSCROLLINFO lpScrollInfo = (LPSCROLLINFO)lParam;
    int nMask = lpScrollInfo->fMask;
    if (nMask & SIF_PAGE)
        lpScrollInfo->nPage = m_si.nPage;
    if (nMask & SIF_POS)
        lpScrollInfo->nPos = m_si.nPos;
    if (nMask & SIF_TRACKPOS)
        lpScrollInfo->nTrackPos = m_si.nTrackPos;
    if (nMask & SIF_RANGE)
    {
        lpScrollInfo->nMin = m_si.nMin;
        lpScrollInfo->nMax = m_si.nMax;
    }
    return TRUE;
}

void SScrollBar::OnDestroy()
{
    m_sbHandler.OnDestroy();
    SWindow::OnDestroy();
}

void SScrollBar::NotifySbCode(int nCode, int nPos)
{
    EventScroll evt(this);
    evt.nSbCode = nCode;
    evt.nPos = nPos;
    evt.bVertical = IsVertical();
    FireEvent(evt);
}

HRESULT SScrollBar::OnAttrVertical(const SStringW &value, BOOL bLoading)
{
    bool bVert = STRINGASBOOL(value);
    m_sbHandler.SetVertical(bVert);
    return bLoading ? S_FALSE : S_OK;
}

void SScrollBar::OnScrollUpdateThumbTrack(BOOL bVert, int nPos)
{
    CRect rcOldThumb = m_sbHandler.GetPartRect(SB_THUMBTRACK);
    m_si.nTrackPos = nPos;
    CRect rcThumb = m_sbHandler.GetPartRect(SB_THUMBTRACK);
    CRect rcUnion;
    rcUnion.UnionRect(rcOldThumb, rcThumb);
    IRenderTarget *pRT = GetRenderTarget(&rcUnion, GRT_PAINTBKGND);
    m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
    m_sbHandler.OnDraw(pRT, SB_THUMBTRACK);
    ReleaseRenderTarget(pRT);
    NotifySbCode(SB_THUMBTRACK, m_si.nTrackPos);
}

void SScrollBar::OnScrollCommand(BOOL bVert, int iCmd, int nPos)
{
    int nOldPos = m_si.nPos;
    switch (iCmd)
    {
    case SB_LINEUP:
        m_si.nPos--;
        break;
    case SB_PAGEUP:
        m_si.nPos -= m_si.nPage;
        break;
    case SB_PAGEDOWN:
        m_si.nPos += m_si.nPage;
        break;
    case SB_LINEDOWN:
        m_si.nPos++;
        break;
    case SB_THUMBPOSITION:
        m_si.nPos = nPos;
        m_si.nTrackPos = -1;
        break;
    }
    if (m_si.nPos < m_si.nMin)
        m_si.nPos = m_si.nMin;
    if (m_si.nPos > m_si.nMax - (int)m_si.nPage + 1)
        m_si.nPos = m_si.nMax - (int)m_si.nPage + 1;
    if (nOldPos != m_si.nPos)
    {
        NotifySbCode(iCmd, m_si.nPos);
        OnScrollUpdatePart(bVert, SB_THUMBTRACK);
    }
}

void SScrollBar::OnScrollSetTimer(BOOL bVert, char id, UINT uElapse)
{
    SetTimer(id, uElapse);
}

void SScrollBar::OnScrollKillTimer(BOOL bVert, char id)
{
    KillTimer(id);
}

CRect SScrollBar::GetScrollBarRect(BOOL bVert) const
{
    return GetClientRect();
}

ISkinObj *SScrollBar::GetScrollBarSkin(BOOL bVert) const
{
    return m_pSkin;
}

const SCROLLINFO *SScrollBar::GetScrollBarInfo(BOOL bVert) const
{
    return &m_si;
}

int SScrollBar::GetScrollBarArrowSize(BOOL bVert) const
{
    return m_uAllowSize;
}

const IInterpolator *SScrollBar::GetScrollInterpolator() const
{
    if (m_fadeFrames > 0 && !m_fadeInterpolator)
    {
        m_fadeInterpolator.Attach(CREATEINTERPOLATOR(L"Accelerate"));
    }
    return m_fadeInterpolator;
}

int SScrollBar::GetScrollFadeFrames() const
{
    return m_fadeFrames;
}

BYTE SScrollBar::GetScrollThumbTrackMinAlpha() const
{
    return m_byThumbTrackMinAlpha;
}

void SScrollBar::OnScrollUpdatePart(BOOL bVert, int iPart)
{
    if (iPart == -1)
    {
        CRect rc = GetScrollBarRect(bVert);
        IRenderTarget *pRT = GetRenderTarget(&rc, GRT_PAINTBKGND);
        m_sbHandler.OnDraw(pRT, SB_LINEUP);
        m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
        m_sbHandler.OnDraw(pRT, SB_THUMBTRACK);
        m_sbHandler.OnDraw(pRT, SB_LINEDOWN);
        ReleaseRenderTarget(pRT);
    }
    else if (iPart == SB_THUMBTRACK)
    {
        CRect rc = m_sbHandler.GetPartRect(SScrollBarHandler::kSbRail);
        IRenderTarget *pRT = GetRenderTarget(&rc, GRT_PAINTBKGND);
        m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
        m_sbHandler.OnDraw(pRT, SB_THUMBTRACK);
        ReleaseRenderTarget(pRT);
    }
    else if (iPart == SB_PAGEDOWN || iPart == SB_PAGEUP)
    {
        CRect rc = m_sbHandler.GetPartRect(iPart);
        IRenderTarget *pRT = GetRenderTarget(&rc, GRT_PAINTBKGND);
        m_sbHandler.OnDraw(pRT, SScrollBarHandler::kSbRail);
        ReleaseRenderTarget(pRT);
    }
    else
    {
        CRect rc = m_sbHandler.GetPartRect(iPart);
        IRenderTarget *pRT = GetRenderTarget(&rc, GRT_PAINTBKGND);
        m_sbHandler.OnDraw(pRT, iPart);
        ReleaseRenderTarget(pRT);
    }
}

ISwndContainer *SScrollBar::GetScrollBarContainer()
{
    return GetContainer();
}

BOOL SScrollBar::IsScrollBarEnable(BOOL bVert) const
{
    return !IsDisabled(TRUE);
}

SNSEND