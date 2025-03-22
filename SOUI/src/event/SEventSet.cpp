﻿#include "souistd.h"
#include <event/SEventSet.h>

SNSBEGIN
//////////////////////////////////////////////////////////////////////////
// SEvent

SEvent::SEvent(DWORD dwEventID, LPCWSTR pszEventName)
    : m_dwEventID(dwEventID)
    , m_strEventName(pszEventName)
{
}

SEvent::~SEvent()
{
    for (UINT i = 0; i < m_evtSlots.GetCount(); i++)
    {
        m_evtSlots[i]->Release();
    }
    m_evtSlots.RemoveAll();
}

BOOL SEvent::subscribe(const IEvtSlot *slot)
{
    if (findSlotFunctor(slot) != -1)
        return false;
    m_evtSlots.Add(slot->Clone());
    return true;
}

BOOL SEvent::unsubscribe(const IEvtSlot *slot)
{
    int idx = findSlotFunctor(slot);
    if (idx == -1)
        return false;

    m_evtSlots[idx]->Release();
    m_evtSlots.RemoveAt(idx);
    return true;
}

int SEvent::findSlotFunctor(const IEvtSlot *slot)
{
    for (UINT i = 0; i < m_evtSlots.GetCount(); i++)
    {
        if (m_evtSlots[i]->Equal(slot))
        {
            return i;
        }
    }
    return -1;
}

void SEvent::fire(IEvtArgs *args)
{
    // execute all subscribers, updating the 'handled' state as we go
    int slots = (int)m_evtSlots.GetCount();
    if (slots == 0)
        return;
    SAutoRefPtr<IEvtSlot> *evtSlots = new SAutoRefPtr<IEvtSlot>[slots];
    for (int i = 0; i < slots; i++)
    {
        evtSlots[i] = m_evtSlots[i];
    }
    for (int i = slots - 1; i >= 0; i--)
    { // the latest event handler handles the event first.
        BOOL bHandled = evtSlots[i]->Run(args);
        if (bHandled)
        {
            args->IncreaseHandleCount();
            if (!args->IsBubbleUp())
                break;
        }
    }
    delete[] evtSlots;
}

void SEvent::SetScriptHandler(const SStringA &strScriptHandler)
{
    m_strScriptHandler = strScriptHandler;
}

SStringA SEvent::GetScriptHandler() const
{
    return m_strScriptHandler;
}

SStringW SEvent::GetName() const
{
    return m_strEventName;
}

DWORD SEvent::GetID()
{
    return m_dwEventID;
}

//////////////////////////////////////////////////////////////////////////
// SEventSet
SEventSet::SEventSet(void)
    : m_nMuted(0)
{
}

SEventSet::~SEventSet(void)
{
    removeAllEvents();
}

void SEventSet::setMutedState(BOOL setting)
{
    if (setting)
        m_nMuted++;
    else
        m_nMuted--;
    SASSERT(m_nMuted >= 0);
}

SEvent *SEventSet::GetEventObject(DWORD dwEventID)
{
    for (UINT i = 0; i < m_evtArr.GetCount(); i++)
    {
        if (m_evtArr[i]->GetID() == dwEventID)
            return m_evtArr[i];
    }
    return NULL;
}

void SEventSet::FireEvent(IEvtArgs *args)
{
    // find event object
    SEvent *ev = GetEventObject(args->GetID());

    // fire the event if present and set is not muted
    if ((ev != 0) && m_nMuted == 0)
    {
        ev->fire(args);
    }
}

BOOL SEventSet::addEvent(DWORD dwEventID, LPCWSTR pszEventHandlerName)
{
    if (isEventPresent(dwEventID))
        return FALSE;
    m_evtArr.Add(new SEvent(dwEventID, pszEventHandlerName));
    return TRUE;
}

BOOL SEventSet::removeEvent(DWORD dwEventID)
{
    for (UINT i = 0; i < m_evtArr.GetCount(); i++)
    {
        if (m_evtArr[i]->GetID() == dwEventID)
        {
            delete m_evtArr[i];
            m_evtArr.RemoveAt(i);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL SEventSet::isEventPresent(DWORD dwEventID)
{
    return GetEventObject(dwEventID) != NULL;
}

void SEventSet::removeAllEvents(void)
{
    for (UINT i = 0; i < m_evtArr.GetCount(); i++)
    {
        delete m_evtArr[i];
    }
    m_evtArr.RemoveAll();
}

BOOL SEventSet::subscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber)
{
    if (!isEventPresent(dwEventID))
        return false;
    return GetEventObject(dwEventID)->subscribe(subscriber);
}

BOOL SEventSet::unsubscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber)
{
    if (!isEventPresent(dwEventID))
        return false;
    return GetEventObject(dwEventID)->unsubscribe(subscriber);
}

#if _MSC_VER >= 1700 // VS2012
BOOL SEventSet::subscribeEvent(DWORD dwEventID, const StdFunCallback &eventCallback)
{
    if (!isEventPresent(dwEventID))
        return false;
    StdFunctionSlot slot(eventCallback);
    return GetEventObject(dwEventID)->subscribe(&slot);
}
#endif

BOOL SEventSet::setEventScriptHandler(const SStringW &strEventName, const SStringA strScriptHandler)
{
    for (UINT i = 0; i < m_evtArr.GetCount(); i++)
    {
        if (m_evtArr[i]->GetName() == strEventName)
        {
            m_evtArr[i]->SetScriptHandler(strScriptHandler);
            return true;
        }
    }
    return false;
}

SStringA SEventSet::getEventScriptHandler(const SStringW &strEventName) const
{
    for (UINT i = 0; i < m_evtArr.GetCount(); i++)
    {
        if (m_evtArr[i]->GetName() == strEventName)
        {
            return m_evtArr[i]->GetScriptHandler();
        }
    }
    return "";
}
SNSEND