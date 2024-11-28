﻿#include "souistd.h"
#include "control/SComboView.h"

SNSBEGIN
SComboView::SComboView(void)
    : m_pListBox(NULL)
{
}

SComboView::~SComboView(void)
{
    if (m_pListBox)
    {
        m_pListBox->SetOwner(NULL);
        m_pListBox->SSendMessage(WM_DESTROY);
        m_pListBox->Release();
    }
}

BOOL SComboView::CreateListBox(SXmlNode xmlNode)
{
    SASSERT(xmlNode);
    //创建列表控件
    SXmlNode listStyle = xmlNode.child(L"listStyle");
    SStringW strListClass = listStyle.attribute(L"wndclass").as_string(SListView::GetClassName());
    m_pListBox = sobj_cast<SListView>(CreateChildByName(strListClass));
    SASSERT(m_pListBox);

    m_pListBox->SetContainer(GetContainer());

    m_pListBox->InitFromXml(&listStyle);
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hotTrack", L"1", TRUE);
    m_pListBox->SetOwner(this); // chain notify message to combobox
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetID(IDC_DROPDOWN_LIST);

    return TRUE;
}

int SComboView::GetListBoxHeight()
{
    int nDropHeight = m_nDropHeight.toPixelSize(GetScale());
    if (GetCount())
    {
        IListViewItemLocator *pItemLocator = m_pListBox->GetItemLocator();
        SASSERT(pItemLocator);
        CRect rcMargin = m_pListBox->GetStyle().GetMargin();
        nDropHeight = smin(nDropHeight, (int)(pItemLocator->GetTotalHeight() + rcMargin.top + rcMargin.bottom));
    }
    return nDropHeight;
}

void SComboView::OnCreateDropDown(SDropDownWnd *pDropDown)
{
    __baseCls::OnCreateDropDown(pDropDown);
    pDropDown->GetRoot()->InsertChild(m_pListBox);
    pDropDown->GetRoot()->UpdateChildrenPosition();

    m_pListBox->SetVisible(TRUE);
    m_pListBox->SetFocus();
    m_pListBox->EnsureVisible(GetCurSel());
}

void SComboView::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    pDropDown->GetRoot()->RemoveChild(m_pListBox);
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetContainer(GetContainer());
    __baseCls::OnDestroyDropDown(pDropDown);
}

void SComboView::OnSelChanged()
{
    m_pListBox->GetSel();
    if (m_pEdit && !m_pEdit->GetEventSet()->isMuted())
    {
        SStringT strText = GetLBText(m_pListBox->GetSel(), FALSE);
        m_pEdit->GetEventSet()->setMutedState(true);
        SComboBase::SetWindowText(strText);
        m_pEdit->GetEventSet()->setMutedState(false);
    }
    Invalidate();
    __baseCls::OnSelChanged();
}

BOOL SComboView::FireEvent(IEvtArgs *evt)
{
    if (evt->IdFrom() == IDC_DROPDOWN_LIST && m_pDropDownWnd)
    {
        if (evt->GetID() == EventLVSelChanged::EventID)
        {
            OnSelChanged();
            return TRUE;
        }
        if (evt->GetID() == EventCmd::EventID)
        {
            CloseUp();
            return TRUE;
        }
    }
    return SComboBase::FireEvent(evt);
}

void SComboView::OnScaleChanged(int nScale)
{
    __baseCls::OnScaleChanged(nScale);
    if (m_pListBox)
        m_pListBox->SSendMessage(UM_SETSCALE, GetScale());
}

SListView *SComboView::GetListView()
{
    return m_pListBox;
}

BOOL SComboView::GetItemText(int iItem, BOOL bRawText, IStringT *str) const
{
    ILvAdapter *pAdapter = m_pListBox->GetAdapter();
    if (!pAdapter || iItem == -1)
        return FALSE;
    SStringW strDesc;
    pAdapter->getItemDesc(iItem, &strDesc);
    if (!bRawText)
        strDesc = tr(strDesc);
    SStringT ret = S_CW2T(strDesc);
    str->Copy(&ret);
    return TRUE;
}

int SComboView::GetCount() const
{
    ILvAdapter *pAdapter = m_pListBox->GetAdapter();
    if (!pAdapter)
        return 0;
    return pAdapter->getCount();
}

int SComboView::GetCurSel() const
{
    return m_pListBox->GetSel();
}

BOOL SComboView::SetCurSel(int iSel)
{
    if (m_pListBox->GetSel() == iSel)
        return FALSE;
    m_pListBox->SetSel(iSel);
    OnSelChanged();
    return TRUE;
}

HRESULT SComboView::OnLanguageChanged()
{
    HRESULT hr = __baseCls::OnLanguageChanged();
    if (m_pListBox)
        m_pListBox->SSendMessage(UM_SETLANGUAGE);
    return hr;
}

IListView *SComboView::GetIListView(THIS)
{
    return GetListView();
}

SNSEND
