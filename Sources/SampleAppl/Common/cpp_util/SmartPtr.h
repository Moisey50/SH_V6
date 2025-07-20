//SmartPtr.h
#ifndef __SMARTPTR_H__
#define __SMARTPTR_H__

#ifndef explicit
	#define explicit 
#endif

template<class T>
class auto_ptr 
{
public:
	explicit auto_ptr(T *p = 0): pointee(p) {}

	auto_ptr( const auto_ptr& rhs ): pointee( rhs.release() ) {}

	~auto_ptr() { delete pointee; }

	auto_ptr<T>& operator=( const auto_ptr& rhs )
	{
		if ( this != &rhs )
			reset( rhs.release() );
		
		return *this;
	}

	operator T*() const { return (T*)pointee; }
	T& operator*() const { return *pointee; }
	T** operator&() { return &pointee; }

	T* operator->() const { return pointee; }

	T* get() const { return pointee; }

	T* release()
	{
		T *oldPointee = pointee;
		pointee = 0;
		return oldPointee;
	}

	void reset(T *p = 0)
	{
		if (pointee != p) 
		{
			delete pointee;
			pointee = p;
		}
	}
  private:
    T *pointee;

  friend class auto_ptr<T>;
};


#endif //__SMARTPTR_H__