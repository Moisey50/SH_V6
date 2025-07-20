#pragma once
#include <string>

#include "rapidjson/prettywriter.h"  // for stringify JSON

using namespace std;
using namespace rapidjson;

#pragma region | Defines |
#pragma endregion | Defines |

template<TBase>
class IJsonContainer
{
#pragma region | Fields |
private:
protected:
public:
#pragma endregion | Fields |

#pragma region | Constructors |

private:

protected:
	IJsonContainer();

	virtual ~IJsonContainer();
public:

#pragma endregion | Constructors |

#pragma region | Methods |
private:
protected:

	virtual template <typename Writer>
		void Serialize(Writer& writer) const = 0;

public:

	virtual TBase* Deserialize(const string& objAsJson) = 0;
	virtual const string& Serialize(const TBase& object) = 0;

	bool TryDeserialize(const string& objAsJson, __out TBase** ppObject)
	{
		TBase* pObj = Deserialize(objAsJson);

		*ppObject = pObj;

		return *ppObject;
	}
#pragma endregion | Methods |
};

