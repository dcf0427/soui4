﻿#include "souistd.h"
#include "layout/SLinearLayout.h"
#include "helper/SplitString.h"
#include <core/SWnd.h>

SNSBEGIN

SLinearLayoutParam::SLinearLayoutParam()
{
    Clear();
}

BOOL SLinearLayoutParam::IsMatchParent(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isMatchParent();
    case Vert:
        return height.isMatchParent();
    case Any:
        return IsMatchParent(Horz) || IsMatchParent(Vert);
    case Both:
    default:
        return IsMatchParent(Horz) && IsMatchParent(Vert);
    }
}

BOOL SLinearLayoutParam::IsWrapContent(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isWrapContent();
    case Vert:
        return height.isWrapContent();
    case Any:
        return IsWrapContent(Horz) || IsWrapContent(Vert);
    case Both:
    default:
        return IsWrapContent(Horz) && IsWrapContent(Vert);
    }
}

BOOL SLinearLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width.isSpecifiedSize();
    case Vert:
        return height.isSpecifiedSize();
    case Any:
        return IsSpecifiedSize(Horz) || IsSpecifiedSize(Vert);
    case Both:
    default:
        return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
    }
}

SLayoutSize SLinearLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
{
    switch (orientation)
    {
    case Horz:
        return width;
    case Vert:
        return height;
    case Any:
    case Both:
    default:
        SASSERT_MSGA(FALSE, "GetSpecifiedSize can only be applied for Horz or Vert");
        return SLayoutSize();
    }
}

HRESULT SLinearLayoutParam::OnAttrSize(const SStringW &strValue, BOOL bLoading)
{
    SStringWList szStr;
    if (2 != SplitString(strValue, L',', szStr))
        return E_FAIL;

    OnAttrWidth(szStr[0], bLoading);
    OnAttrHeight(szStr[1], bLoading);
    return S_OK;
}

HRESULT SLinearLayoutParam::OnAttrWidth(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0)
        width.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        width.setWrapContent();
    else
        width = GETLAYOUTSIZE(strValue);
    return S_OK;
}

HRESULT SLinearLayoutParam::OnAttrHeight(const SStringW &strValue, BOOL bLoading)
{
    if (strValue.CompareNoCase(L"matchParent") == 0)
        height.setMatchParent();
    else if (strValue.CompareNoCase(L"wrapContent") == 0)
        height.setWrapContent();
    else
        height = GETLAYOUTSIZE(strValue);
    return S_OK;
}

HRESULT SLinearLayoutParam::OnAttrExtend(const SStringW &strValue, BOOL bLoading)
{
    SStringWList strList;
    size_t nSeg = SplitString(strValue, L',', strList);
    if (nSeg == 1)
    {
        extend_left = extend_top = extend_right = extend_bottom = GETLAYOUTSIZE(strList[0]);
        return S_OK;
    }
    else if (nSeg == 4)
    {
        extend_left = GETLAYOUTSIZE(strList[0]);
        extend_top = GETLAYOUTSIZE(strList[1]);
        extend_right = GETLAYOUTSIZE(strList[2]);
        extend_bottom = GETLAYOUTSIZE(strList[3]);
        return S_OK;
    }
    return E_INVALIDARG;
}

void SLinearLayoutParam::Clear()
{
    width.setWrapContent();
    height.setWrapContent();
    weight = 0.0f;
    gravity = G_Undefined;
}

void SLinearLayoutParam::SetMatchParent(ORIENTATION orientation)
{
    switch (orientation)
    {
    case Horz:
        width.setMatchParent();
        break;
    case Vert:
        height.setMatchParent();
        break;
    case Both:
        width.setMatchParent();
        height.setMatchParent();
        break;
    }
}

void SLinearLayoutParam::SetWrapContent(ORIENTATION orientation)
{
    switch (orientation)
    {
    case Horz:
        width.setWrapContent();
        break;
    case Vert:
        height.setWrapContent();
        break;
    case Both:
        width.setWrapContent();
        height.setWrapContent();
        break;
    }
}

void SLinearLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const SLayoutSize &layoutSize)
{
    switch (orientation)
    {
    case Horz:
        width = layoutSize;
        break;
    case Vert:
        height = layoutSize;
        break;
    case Both:
        width = height = layoutSize;
        break;
    }
}

void *SLinearLayoutParam::GetRawData()
{
    return (SLinearLayoutParamStruct *)this;
}

ILayoutParam *SLinearLayoutParam::Clone() const
{
    SLinearLayoutParam *pRet = new SLinearLayoutParam();
    memcpy(pRet->GetRawData(), (SLinearLayoutParamStruct *)this, sizeof(SLinearLayoutParamStruct));
    return pRet;
}

//////////////////////////////////////////////////////////////////////////
SLinearLayout::SLinearLayout(void)
    : m_gravity(G_Undefined)
{
}

SLinearLayout::~SLinearLayout(void)
{
}

void SLinearLayout::LayoutChildren(IWindow *pParent)
{
    CRect rcParent;
    pParent->GetChildrenLayoutRect(&rcParent);

    SIZE *pSize = new SIZE[pParent->GetChildrenCount()];
    IWindow **pChilds = new IWindow *[pParent->GetChildrenCount()];
    memset(pSize, 0, sizeof(SIZE) * pParent->GetChildrenCount());

    int nChilds = 0;

    int offset = 0;
    float fWeight = 0.0f;

    { // assign width or height

        int iChild = 0;

        IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
        while (pChild)
        {
            const SLinearLayoutParam *pLinearLayoutParam = (const SLinearLayoutParam *)pChild->GetLayoutParam();

            int nScale = pChild->GetScale();

            CSize szChild(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
            if (pLinearLayoutParam->IsMatchParent(Horz))
                szChild.cx = rcParent.Width();
            else if (pLinearLayoutParam->IsSpecifiedSize(Horz))
            {
                szChild.cx = pLinearLayoutParam->GetSpecifiedSize(Horz).toPixelSize(nScale);
                szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
            }

            if (pLinearLayoutParam->IsMatchParent(Vert))
                szChild.cy = rcParent.Height();
            else if (pLinearLayoutParam->IsSpecifiedSize(Vert))
            {
                szChild.cy = pLinearLayoutParam->GetSpecifiedSize(Vert).toPixelSize(nScale);
                szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
            }

            if (pLinearLayoutParam->weight > 0.0f)
            {
                fWeight += pLinearLayoutParam->weight;
            }

            if (szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
            {
                CSize szCalc;
                pChild->GetDesiredSize(&szCalc, szChild.cx, szChild.cy);
                if (szChild.cx == SIZE_WRAP_CONTENT)
                {
                    szChild.cx = szCalc.cx;
                    szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                }
                if (szChild.cy == SIZE_WRAP_CONTENT)
                {
                    szChild.cy = szCalc.cy;
                    szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                }
            }

            pChilds[iChild] = pChild;
            pSize[iChild] = szChild;
            offset += m_orientation == Vert ? szChild.cy : szChild.cx;

            iChild++;
            pChild = pParent->GetNextLayoutIChild(pChild);
        }

        nChilds = iChild;
    }

    int size = m_orientation == Vert ? rcParent.Height() : rcParent.Width();
    int interval = m_interval.toPixelSize(pParent->GetScale());
    if (interval > 0)
    {
        offset += (nChilds - 1) * interval;
    }
    ORIENTATION orienOther = m_orientation == Vert ? Horz : Vert;
    if (size > offset)
    {
        if (fWeight > 0.0f)
        { // assign size by weight
            int nRemain = size - offset;

            for (int iChild = 0; iChild < nChilds; iChild++)
            {
                if (SLayoutSize::fequal(fWeight, 0.0f))
                    break;
                IWindow *pChild = pChilds[iChild];
                SLinearLayoutParam *pLinearLayoutParam = (SLinearLayoutParam *)pChild->GetLayoutParam();
                int nScale = pChild->GetScale();
                if (pLinearLayoutParam->weight > 0.0f)
                {
                    int extra = int(nRemain * pLinearLayoutParam->weight / fWeight + 0.5f);
                    LONG &szChild = m_orientation == Vert ? pSize[iChild].cy : pSize[iChild].cx;
                    szChild += extra;
                    nRemain -= extra;
                    fWeight -= pLinearLayoutParam->weight;

                    if (pLinearLayoutParam->IsWrapContent(orienOther))
                    {
                        ILayoutParam *backup = pLinearLayoutParam->Clone();
                        pLinearLayoutParam->SetSpecifiedSize(m_orientation, SLayoutSize((float)szChild, SLayoutSize::dp));
                        int nWid = pSize[iChild].cx, nHei = pSize[iChild].cy;

                        if (orienOther == Vert)
                            nHei = SIZE_WRAP_CONTENT;
                        else
                            nWid = SIZE_WRAP_CONTENT;

                        CSize szCalc;
                        pChild->GetDesiredSize(&szCalc, nWid, nHei);
                        if (orienOther == Vert)
                        {
                            szCalc.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                            pSize[iChild].cy = szCalc.cy;
                        }
                        else
                        {
                            szCalc.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                            pSize[iChild].cx = szCalc.cx;
                        }
                        pChild->SetLayoutParam(backup);
                        backup->Release();
                    }
                }
            }
        }
        else if (interval < 0)
        {
            // handle interval==-1 or interval==-2
            if (interval == SIZE_WRAP_CONTENT && nChilds > 1)
                interval = (size - offset) / (nChilds - 1);
            else if (interval == SIZE_MATCH_PARENT)
            {
                interval = (size - offset) / (nChilds + 1);
                if (m_orientation == Vert)
                    rcParent.DeflateRect(0, interval, 0, interval);
                else
                    rcParent.DeflateRect(interval, 0, interval, 0);
            }
            else
                interval = 0;
        }
    }
    { // assign position
        offset = 0;
        for (int iChild = 0; iChild < nChilds; iChild++)
        {
            SWindow *pChild = (SWindow *)pChilds[iChild];

            SLinearLayoutParam *pLinearLayoutParam = (SLinearLayoutParam *)pChild->GetLayoutParam();
            int nScale = pChild->GetScale();
            Gravity gravity = pLinearLayoutParam->gravity == G_Undefined ? m_gravity : pLinearLayoutParam->gravity;
            if (gravity == G_Undefined)
                gravity = G_Left;

            if (m_orientation == Vert)
            {
                CRect rcChild(CPoint(0, offset), pSize[iChild]);
                rcChild.OffsetRect(rcParent.TopLeft());
                if (gravity == G_Center)
                    rcChild.OffsetRect((rcParent.Width() - rcChild.Width()) / 2, 0);
                else if (gravity == G_Right)
                    rcChild.OffsetRect(rcParent.Width() - rcChild.Width(), 0);

                CRect rcChild2 = rcChild;
                rcChild2.DeflateRect(pLinearLayoutParam->extend_left.toPixelSize(nScale), pLinearLayoutParam->extend_top.toPixelSize(nScale), pLinearLayoutParam->extend_right.toPixelSize(nScale), pLinearLayoutParam->extend_bottom.toPixelSize(nScale));

                pChild->OnRelayout(rcChild2);

                offset += rcChild.Height();
            }
            else
            {
                CRect rcChild(CPoint(offset, 0), pSize[iChild]);
                rcChild.OffsetRect(rcParent.TopLeft());
                if (gravity == G_Center)
                    rcChild.OffsetRect(0, (rcParent.Height() - rcChild.Height()) / 2);
                else if (gravity == G_Right)
                    rcChild.OffsetRect(0, rcParent.Height() - rcChild.Height());

                CRect rcChild2 = rcChild;
                rcChild2.DeflateRect(pLinearLayoutParam->extend_left.toPixelSize(nScale), pLinearLayoutParam->extend_top.toPixelSize(nScale), pLinearLayoutParam->extend_right.toPixelSize(nScale), pLinearLayoutParam->extend_bottom.toPixelSize(nScale));

                pChild->OnRelayout(rcChild2);

                offset += rcChild.Width();
            }
            offset += interval;
        }
    }

    delete[] pChilds;
    delete[] pSize;
}

// nWidth,nHeight == -1:wrap_content
SIZE SLinearLayout::MeasureChildren(const IWindow *pParent, int nWidth, int nHeight) const
{
    int nChildCount = pParent->GetChildrenCount();
    SIZE *pSize = new SIZE[nChildCount];
    memset(pSize, -1, sizeof(SIZE) * nChildCount); // init size to -1
    const IWindow **ppChilds = new const IWindow *[nChildCount];
    memset(ppChilds, 0, sizeof(IWindow *) * nChildCount);
    const ILayoutParam *pParentLayoutParam = (const ILayoutParam *)pParent->GetLayoutParam();

    bool bWaitingForLimit = true;
    float fWeight = 0;
    int nWaiting = 0;
    int nChilds = 0;

measureChilds:
{
    int iChild = 0;
    const IWindow *pChild = pParent->GetNextLayoutIChild(NULL);
    while (pChild)
    {
        while (!ppChilds[iChild])
        {
            const SLinearLayoutParam *pLinearLayoutParam = (const SLinearLayoutParam *)pChild->GetLayoutParam();
            int nScale = pChild->GetScale();
            CSize szChild(SIZE_WRAP_CONTENT, SIZE_WRAP_CONTENT);
            if (pLinearLayoutParam->IsMatchParent(Horz))
            {
                if (!pParentLayoutParam->IsWrapContent(Horz))
                {
                    szChild.cx = nWidth;
                }
                else if (bWaitingForLimit && nWidth == SIZE_WRAP_CONTENT && m_orientation == Vert)
                {
                    nWaiting++;
                    break;
                }
            }
            else if (pLinearLayoutParam->IsSpecifiedSize(Horz))
            {
                szChild.cx = pLinearLayoutParam->GetSpecifiedSize(Horz).toPixelSize(nScale);
                szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
            }
            if (pLinearLayoutParam->IsMatchParent(Vert))
            {
                if (!pParentLayoutParam->IsWrapContent(Vert))
                {
                    szChild.cy = nHeight;
                }
                else if (bWaitingForLimit && nHeight == SIZE_WRAP_CONTENT && m_orientation == Horz)
                {
                    nWaiting++;
                    break;
                }
            }
            else if (pLinearLayoutParam->IsSpecifiedSize(Vert))
            {
                szChild.cy = pLinearLayoutParam->GetSpecifiedSize(Vert).toPixelSize(nScale);
                szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
            }
            if (szChild.cx == SIZE_WRAP_CONTENT || szChild.cy == SIZE_WRAP_CONTENT)
            {
                int nWid = szChild.cx, nHei = szChild.cy;

                CSize szCalc;
                ((IWindow *)pChild)->GetDesiredSize(&szCalc, nWid, nHei);
                if (szChild.cx == SIZE_WRAP_CONTENT)
                {
                    szChild.cx = szCalc.cx;
                    szChild.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                }
                if (szChild.cy == SIZE_WRAP_CONTENT)
                {
                    szChild.cy = szCalc.cy;
                    szChild.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                }
            }
            fWeight += pLinearLayoutParam->weight;

            ppChilds[iChild] = pChild;
            pSize[iChild] = szChild;
            break;
        }
        pChild = pParent->GetNextLayoutIChild(pChild);
        iChild++;
    }
    nChilds = iChild;
}
    if (nWaiting > 0)
    {
        //父窗口的非布局方向为自适应，同时存在子窗口在该方向上为填充父窗口，需要计算出该方向其它子窗口的最大值。
        nWaiting = 0;
        if (m_orientation == Vert)
        {
            SASSERT(nWidth == SIZE_WRAP_CONTENT);
            for (int i = 0; i < nChilds; i++)
            {
                nWidth = smax(pSize[i].cx, nWidth);
            }
            bWaitingForLimit = false;
            nChilds = 0;
            goto measureChilds;
        }
        else
        {
            SASSERT(nHeight == SIZE_WRAP_CONTENT);
            for (int i = 0; i < nChilds; i++)
            {
                nHeight = smax(pSize[i].cy, nHeight);
            }
            bWaitingForLimit = false;
            nChilds = 0;
            goto measureChilds;
        }
    }

    int size = m_orientation == Vert ? nHeight : nWidth;
    int nInterval = m_interval.toPixelSize(pParent->GetScale());
    if (nInterval < 0)
        nInterval = 0;
    if (!SLayoutSize::fequal(fWeight, 0.0f) && size != SIZE_WRAP_CONTENT)
    { // assign weight for elements. and calc size of other orientation again.
        int offset = 0;
        for (int i = 0; i < nChilds; i++)
        {
            offset += m_orientation == Vert ? pSize[i].cy : pSize[i].cx;
        }
        offset += nInterval * (nChilds - 1);
        if (offset < size)
        { // assign size by weight
            int nRemain = size - offset;

            ORIENTATION orienOther = m_orientation == Vert ? Horz : Vert;
            for (int iChild = 0; iChild < nChilds; iChild++)
            {
                if (SLayoutSize::fequal(fWeight, 0.0f) && pSize[iChild].cx != -1 && pSize[iChild].cy != -1)
                    break;
                IWindow *pChild = (IWindow *)ppChilds[iChild];
                SLinearLayoutParam *pLinearLayoutParam = (SLinearLayoutParam *)pChild->GetLayoutParam();
                int nScale = pChild->GetScale();
                if (pLinearLayoutParam->weight > 0.0f)
                {
                    int extra = int(nRemain * pLinearLayoutParam->weight / fWeight + 0.5f);
                    LONG &szChild = m_orientation == Vert ? pSize[iChild].cy : pSize[iChild].cx;
                    szChild += extra;
                    nRemain -= extra;
                    fWeight -= pLinearLayoutParam->weight;

                    if (!pLinearLayoutParam->IsSpecifiedSize(orienOther))
                    { // As pChild->GetDesiredSize may use layout param to get specified size, we
                      // must set it to new size.
                        ILayoutParam *backup = pLinearLayoutParam->Clone();
                        pLinearLayoutParam->SetSpecifiedSize(m_orientation, SLayoutSize((float)szChild, SLayoutSize::dp));
                        pLinearLayoutParam->SetWrapContent(orienOther);
                        int nWid = pSize[iChild].cx, nHei = pSize[iChild].cy;

                        if (orienOther == Vert)
                            nHei = SIZE_WRAP_CONTENT;
                        else
                            nWid = SIZE_WRAP_CONTENT;

                        CSize szCalc;
                        pChild->GetDesiredSize(&szCalc, nWid, nHei);
                        if (orienOther == Vert)
                        {
                            szCalc.cy += pLinearLayoutParam->extend_top.toPixelSize(nScale) + pLinearLayoutParam->extend_bottom.toPixelSize(nScale);
                            pSize[iChild].cy = szCalc.cy;
                        }
                        else
                        {
                            szCalc.cx += pLinearLayoutParam->extend_left.toPixelSize(nScale) + pLinearLayoutParam->extend_right.toPixelSize(nScale);
                            pSize[iChild].cx = szCalc.cx;
                        }
                        pChild->SetLayoutParam(backup);
                        backup->Release();
                    }
                }
            }
        }
    }

    CSize szRet;
    for (int i = 0; i < nChilds; i++)
    {
        if (m_orientation == Horz)
        {
            szRet.cy = smax(szRet.cy, pSize[i].cy);
            szRet.cx += pSize[i].cx;
        }
        else
        {
            szRet.cx = smax(szRet.cx, pSize[i].cx);
            szRet.cy += pSize[i].cy;
        }
    }
    // add intervals
    if (m_orientation == Horz)
    {
        szRet.cx += nInterval * (nChilds - 1);
    }
    else
    {
        szRet.cy += nInterval * (nChilds - 1);
    }
    delete[] pSize;
    delete[] ppChilds;
    return szRet;
}

BOOL SLinearLayout::IsParamAcceptable(const ILayoutParam *pLayoutParam) const
{
    return pLayoutParam->IsClass(SLinearLayoutParam::GetClassName());
}

ILayoutParam *SLinearLayout::CreateLayoutParam() const
{
    return new SLinearLayoutParam();
}

SNSEND
