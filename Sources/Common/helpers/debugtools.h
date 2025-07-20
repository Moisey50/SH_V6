#pragma once

#define DO_CHK_HEAP

#ifdef _DEBUG
    #ifdef DO_CHK_HEAP
        class chk_heap
        {
        public:
            chk_heap()
            {
                ASSERT(_heapchk()==_HEAPOK);
            }
            ~chk_heap()
            {
                ASSERT(_heapchk()==_HEAPOK);
            }
        };
        #define CHK_HEAP chk_heap checkheap
    #else
        #define CHK_HEAP
    #endif
#else
    #define CHK_HEAP
#endif

#ifdef _DEBUG
    #ifdef DO_CHK_HEAP
        inline bool valid_pntr(LPCVOID pntr)
        {
        /*    _HEAPINFO hinfo;
            int heapstatus;
            hinfo._pentry = NULL;
            while((heapstatus = _heapwalk(&hinfo)) == _HEAPOK)
            {
                if ((pntr>=hinfo._pentry) && (pntr<hinfo._pentry+hinfo._size))
                {
                    TRACE(">>> valid_pntr(0x%x) returns %s (heapentry: 0x%x)\n",pntr,(hinfo._useflag == _USEDENTRY)?"true":"false",hinfo._pentry);
                    return hinfo._useflag == _USEDENTRY;
                }
            } 
            return false; */
            int _prefix;
            __try 
            {
                _prefix=*(((char*)pntr)-1); //Get the prefix of this data
            } __except (true) 
            { //Catch all unique exceptions (Windows exceptions) 
                return false; //Can't reach this memory
            }
            TRACE(">>> %d\n",_prefix);
            switch (_prefix) 
            {
            //case 0:    //Running release mode with debugger
            //case -128: //Running release mode without debugger
            case -2:   //Running debug mode with debugger
            //case -35:  //Running debug mode without debugger
                return false; //Deleted :(
            default:
                break;
            } 
            return true;
        }
    #else
        #define valid_pntr(A) true 
    #endif
#else
    #define valid_pntr(A) true
#endif
