#ifndef _IENUMERATOR_H_
#define _IENUMERATOR_H_

#include <vector>
#include <memory>

using namespace std;

namespace DeviceDetectLibrary
{
    class ICollector;
    struct DeviceInfo;


    class IEnumerator //non copyable
    {
		DISALLOW_COPY_AND_ASSIGN(IEnumerator);

    public:
		IEnumerator(){};
		virtual ~IEnumerator() {}
        
		virtual void Collect(const DeviceInfo& ) = 0;
    };

    typedef std::shared_ptr<IEnumerator> IEnumerator_Ptr;
    typedef vector<IEnumerator_Ptr>      IEnumerators_vt;
}

#endif // _IENUMERATOR_H_