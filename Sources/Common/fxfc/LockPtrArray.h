#if !defined(LOCKPTRARRAY_INCLUDED_)
#define LOCKPTRARRAY_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FXLockPtrArray: public FXPtrArray
{
private:
    FXLockObject m_Lock;
public:
    // direct locking proc
    void    LockPtr() { m_Lock.Lock(); }
    void    UnlockPtr() { m_Lock.Unlock(); }
    FXSIZE     GetSizeLocked() { FXAutolock lock(m_Lock); return GetSize(); };
    FXSIZE     AddLocked(void* element) { FXAutolock lock(m_Lock); return Add(element); };
    bool    RemoveElement(void* element)
    {
        for ( FXSIZE i = 0; i < GetSize(); i++)
        {
            if (GetAt(i) == element)
            {
                RemoveAt(i);
                return true;
            }
        }
        return false;
    };
    FXSIZE     CopyLocked(FXPtrArray& Copy)
    { 
        FXAutolock lock(m_Lock); 
        Copy.RemoveAll(); 
        Copy.Append(*this); 
        return GetSize(); 
    };
};

#endif //#if !defined(LOCKPTRARRAY_INCLUDED_)
