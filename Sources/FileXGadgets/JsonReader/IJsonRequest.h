#pragma once
#include "IJsonContainer.h"

#pragma region | Defines |


#pragma endregion | Defines |

template<typename TBase>
class IJsonRequest :
	public IJsonContainer<TBase>
{
#pragma region | Fields |
private:
	bool m_requestToWrite;
protected:
public:

#pragma endregion | Fields |

#pragma region | Constructors |

private:

protected:
	IJsonRequest();

	virtual ~IJsonContainer();
public:

#pragma endregion | Constructors |

#pragma region | Methods |
private:
protected:
	virtual TBase* OnDeserialize(const string& objAsJson) = 0;
	virtual const string& OnSerialize(const TBase& object) = 0;

public:
	const string& GetRequestType() const
	{
		return m_requestType;
	}

	override TBase* Deserialize(const string& objAsJson)
	{
		return OnDeserialize(objAsJson);
	}
	override const string& Serialize(const TBase& object)
	{
		return OnSerialize(object);
	}

#pragma endregion | Methods |
};

