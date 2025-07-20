// StaticQueue.h: interface for the FXStaticQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATICQUEUE_H__D666048B_C30D_4A34_9B04_0579993CD5BF__INCLUDED_)
#define AFX_STATICQUEUE_H__D666048B_C30D_4A34_9B04_0579993CD5BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_SQ_QUEUESIZE   4

template <class T> class FXStaticQueue 
{
private:
    HANDLE          m_hSemPut;        // Semaphore controlling queue "putting"
    HANDLE          m_hSemGet;        // Semaphore controlling queue "getting"
    CRITICAL_SECTION m_CritSect;      // Thread seriallization
    int             m_nMax;           // Max objects allowed in queue
    volatile int    m_iNextPut;       // Array index of next "PutMsg"
    volatile int    m_iNextGet;       // Array index of next "GetMsg"
    long            m_iInQueue;
    T              *m_QueueObjects;   // Array of objects (ptr's to void)

    void Initialize(int n) 
    {
        m_iNextPut = m_iNextGet = 0;
        m_nMax = n;
        m_iInQueue=0;
        if ( m_QueueObjects )
        {
          delete[] m_QueueObjects;
          m_QueueObjects = NULL ;
        }
        m_QueueObjects = new T[n];
        ::InitializeCriticalSection(&m_CritSect);
        m_hSemPut = ::CreateSemaphore(NULL, n, n, NULL);
        m_hSemGet = ::CreateSemaphore(NULL, 0, n, NULL);
    }
public:
    FXStaticQueue(int n=DEFAULT_SQ_QUEUESIZE) 
    {
      m_QueueObjects = NULL ;
      Initialize(n);
    }

    ~FXStaticQueue() 
    {
        ::EnterCriticalSection(&m_CritSect);
        ASSERT(m_iInQueue==0);
        delete [] m_QueueObjects; 
        m_QueueObjects=NULL;
        ::DeleteCriticalSection(&m_CritSect);
        ::CloseHandle(m_hSemPut);
        ::CloseHandle(m_hSemGet);
    }
    
    int GetQueueSize()
    {
        return m_nMax;
    }

    void ClearQueue()
    {
      ::EnterCriticalSection( &m_CritSect );
      // 1. Clean all possible get requests
      while ( ::WaitForSingleObject( m_hSemGet , 0 ) == WAIT_OBJECT_0 ) ;
      // 2. Fill full put request queue
      LONG lWas = 0 ;
      ReleaseSemaphore( m_hSemPut , m_nMax , &lWas ) ;
      // 3. Reset all indexes.
      m_iNextPut = m_iNextGet = m_iInQueue = 0 ;
      ::LeaveCriticalSection( &m_CritSect );
    }
    bool ResizeQueue(int size)
    {
        ::EnterCriticalSection(&m_CritSect);
        if (m_iInQueue>=size)
        {
            ::LeaveCriticalSection(&m_CritSect);
            return false;
        }
        T* newQueue = new T[size];
        int iNextPut = 0 , iNextGet = 0 ;
        int iInQueue=0;
        int iSlot , iSlotN;
        while(m_iInQueue)
        {
            iSlot = m_iNextGet++ % m_nMax;
            iSlotN = iNextPut++ % size;
            newQueue[iSlotN] = m_QueueObjects[iSlot];
            ::InterlockedDecrement(&m_iInQueue);
            iInQueue++;
        }
        delete [] m_QueueObjects;
        m_QueueObjects=newQueue;
        ::InterlockedExchange(&m_iInQueue,iInQueue);
        m_iNextPut=iNextPut;
        m_iNextGet=iNextGet;
        ::LeaveCriticalSection(&m_CritSect);
        return true;
    }

    int ItemsInQueue()
    {
        ::EnterCriticalSection(&m_CritSect);
        int i = m_iInQueue;
        ::LeaveCriticalSection(&m_CritSect);
        return i;
    }

    bool GetQueueObject(T& Object) 
    {
        int iSlot;
        LONG lPrevious;

		//TRACE(" STATIC QUEUE 0x%x GET:", this);
        if (::WaitForSingleObject(m_hSemGet, 0)==WAIT_OBJECT_0)
        {
            ::EnterCriticalSection(&m_CritSect);
            iSlot = m_iNextGet++ % m_nMax;
            Object = m_QueueObjects[iSlot];
            ::InterlockedDecrement(&m_iInQueue);
            ::ReleaseSemaphore(m_hSemPut, 1L, &lPrevious);
            ::LeaveCriticalSection(&m_CritSect);
			//TRACE(" SUCCEED\n");
            return true;
        }
		//TRACE(" FAILED\n");
        return false;
    }

    bool PutQueueObject(T Object) 
    {
        int iSlot;
        LONG lPrevious;

		//TRACE(" STATIC QUEUE 0x%x PUT:", this);
        if (::WaitForSingleObject(m_hSemPut, 0)==WAIT_OBJECT_0)
        {
            ::EnterCriticalSection(&m_CritSect);
            iSlot = m_iNextPut++ % m_nMax;
            m_QueueObjects[iSlot] = Object;
            ::InterlockedIncrement(&m_iInQueue);
            ::ReleaseSemaphore(m_hSemGet, 1L, &lPrevious);
            ::LeaveCriticalSection(&m_CritSect);
			//TRACE(" SUCCEED\n");
            return true;
        }
		//TRACE(" FAILED\n");
        return false;
    }

    bool Peep(int offset, T& Object)
    {
        int iSlot = -1 ;
        ::EnterCriticalSection(&m_CritSect);
        if ( m_iInQueue > offset )
        {
          iSlot = (m_iNextGet+offset) % m_nMax;
          Object = m_QueueObjects[iSlot];
        }
        ::LeaveCriticalSection(&m_CritSect);
        return iSlot >= 0 ;
    }
};


//// STACK

template <class T> class FXStaticStack 
{
private:
    HANDLE          m_hSemPut;        // Semaphore controlling queue "putting"
    HANDLE          m_hSemGet;        // Semaphore controlling queue "getting"
    CRITICAL_SECTION m_CritSect;      // Thread serialization
    int             m_nMax;           // Max objects allowed in stack
    volatile long   m_iTop;			  // Array index of next "PutMsg"
    T              *m_StackObjects;   // Array of objects (ptrs to void)

    void Initialize(int n) 
    {
        m_iTop = 0;
        m_nMax = n;
        m_StackObjects = new T[n];
        ::InitializeCriticalSection(&m_CritSect);
        m_hSemPut = ::CreateSemaphore(NULL, n, n, NULL);
        m_hSemGet = ::CreateSemaphore(NULL, 0, n, NULL);
    }
public:
    FXStaticStack(int n=DEFAULT_SQ_QUEUESIZE) 
    {
        Initialize(n);
    }

    ~FXStaticStack() 
    {
        ::EnterCriticalSection(&m_CritSect);
        ASSERT(m_iTop==0);
        delete [] m_StackObjects; m_StackObjects=NULL;
        ::DeleteCriticalSection(&m_CritSect);
        ::CloseHandle(m_hSemPut);
        ::CloseHandle(m_hSemGet);
    }
    
    int ItemsInStack()
    {
        ::EnterCriticalSection(&m_CritSect);
        int i = (int)m_iTop;
        ::LeaveCriticalSection(&m_CritSect);
        return i;
    }

    bool GetStackObject(T& Object) 
    {
        int iSlot;
        LONG lPrevious;

		//TRACE(" STATIC QUEUE 0x%x GET:", this);
        if (::WaitForSingleObject(m_hSemGet, 0)==WAIT_OBJECT_0)
        {
            ::EnterCriticalSection(&m_CritSect);
            ::InterlockedDecrement(&m_iTop);
            iSlot = (int)m_iTop;
			if (iSlot >= 0)
			{
				Object = m_StackObjects[iSlot];
				::ReleaseSemaphore(m_hSemPut, 1L, &lPrevious);
				::LeaveCriticalSection(&m_CritSect);
				//TRACE(" SUCCEED\n");
				return true;
			}
			else
			{
	            ::InterlockedIncrement(&m_iTop);
				::ReleaseSemaphore(m_hSemPut, 1L, &lPrevious);
				::LeaveCriticalSection(&m_CritSect);
				return false;
			}
        }
		//TRACE(" FAILED\n");
        return false;
    }

    bool PutStackObject(T Object) 
    {
        int iSlot;
        LONG lPrevious;

		//TRACE(" STATIC QUEUE 0x%x PUT:", this);
        if (::WaitForSingleObject(m_hSemPut, 0)==WAIT_OBJECT_0)
        {
            ::EnterCriticalSection(&m_CritSect);
			iSlot = (int)m_iTop;
			if (iSlot < m_nMax)
			{
				m_StackObjects[iSlot] = Object;
				::InterlockedIncrement(&m_iTop);
				::ReleaseSemaphore(m_hSemGet, 1L, &lPrevious);
				::LeaveCriticalSection(&m_CritSect);
				//TRACE(" SUCCEED\n");
				return true;
			}
			else
			{
				::ReleaseSemaphore(m_hSemGet, 1L, &lPrevious);
				::LeaveCriticalSection(&m_CritSect);
				return false;
			}
        }
		//TRACE(" FAILED\n");
        return false;
    }

    bool Peep(int offset, T& Object)
    {
        int iSlot;
        ::EnterCriticalSection(&m_CritSect);
        if (m_iTop<=offset) { ::LeaveCriticalSection(&m_CritSect); return false; }
        iSlot = ((int)m_iTop-offset-1);
        Object = m_StackObjects[iSlot];
        ::LeaveCriticalSection(&m_CritSect);
        return true;
    }
};



#endif // !defined(AFX_STATICQUEUE_H__D666048B_C30D_4A34_9B04_0579993CD5BF__INCLUDED_)
