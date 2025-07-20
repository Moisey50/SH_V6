#pragma once
#include <string>
#include <vector>
#include <algorithm>

#include <sstream>
#include <fstream> //file read

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"  // for stringify JSON

using namespace std;
using namespace rapidjson;

typedef
void (*LPLogMsgHandler)(const string& message, bool isError);


#pragma region | Defines |

#define JF_KEY_ID ("id")
#define JF_KEY_X  ("X")
#define JF_KEY_Y  ("Y")
#define JF_KEY_Z  ("Z")
#define JF_KEY_dX ("dX")
#define JF_KEY_dY ("dY")
#define JF_KEY_dZ ("dZ")

#pragma endregion | Defines |

class JFigure	
{
#pragma region | Fields |
private:
	string m_id;
	double m_x;
	double m_y;
	double m_z;
	double m_dx;
	double m_dy;
	double m_dz;
protected:
public:

#pragma endregion | Fields |

#pragma region | Constructors |

private:

protected:

public:
	JFigure(const string& id)
		: m_id(id)
		, m_x(0.)
		, m_y(0.)
		, m_z(0.)
		, m_dx(0.)
		, m_dy(0.)
		, m_dz(0.)
	{
	}

	virtual ~JFigure()
	{
	}

#pragma endregion | Constructors |

#pragma region | Methods |
private:

	//template<typename T>
	void AssignValue(const Value& jsonDocument, const string& key, string& value, LPLogMsgHandler messageHandler)
	{
		if (jsonDocument.HasMember(key.c_str()))
		{
			const Value& jv = jsonDocument[key.c_str()];
			
			if (jv.IsString())
				value = jv.GetString();
			else
			{
				stringstream ss;

				ss << "Wrong value for the '" << key << "' key" << std::endl;
				messageHandler(ss.str(), true);
			}
		}
	}

	void AssignValue(const Value& jsonDocument, const string& key, double& value, LPLogMsgHandler messageHandler)
	{
		if (jsonDocument.HasMember(key.c_str()) && jsonDocument[key.c_str()].IsNumber())
		{
			value = jsonDocument[key.c_str()].GetDouble();
		}
		else
		{
			string val;
			AssignValue(jsonDocument, key, val, messageHandler);

			value = std::strtod(val.c_str(), NULL);
		}
	}
protected:
public:

	template <typename Writer>
	void Serialize(Writer& writer) const {
		
		writer.StartObject();
		// This base class just write out name-value pairs, without wrapping within an object.
		writer.String(JF_KEY_ID);

#if RAPIDJSON_HAS_STDSTRING
		writer.String(m_id);
#else
		writer.String(m_id.c_str(), static_cast<SizeType>(m_id.length())); // Supplying length of string is faster.
#endif
		writer.String(JF_KEY_X);
		writer.Double(m_x);
		writer.String(JF_KEY_Y);
		writer.Double(m_y);

		if (m_z != 0 || m_dz != 0)
		{
			writer.String(JF_KEY_Z);
			writer.Double(m_z);
		}

		if (m_dx != 0 || m_dy != 0 || m_dz != 0)
		{
			writer.String(JF_KEY_dX);
			writer.Double(m_dx);
		}

		if (m_dy != 0 || m_dz != 0)
		{
			writer.String(JF_KEY_dY);
			writer.Double(m_dy);
		}

		if (m_dz != 0)
		{
			writer.String(JF_KEY_dZ);
			writer.Double(m_dz);
		}
		writer.EndObject();
	}


	const string& GetId() const
	{
		return m_id;
	}

	const double GetX() const
	{
		return m_x;
	}
	JFigure& SetX(double x)
	{
		m_x = x;
		return *this;
	}

	const double GetY() const
	{
		return m_y;
	}
	JFigure& SetY(double y)
	{
		m_y = y;
		return *this;
	}

	const double GetZ() const
	{
		return m_z;
	}
	JFigure& SetZ(double z)
	{
		m_z = z;
		return *this;
	}


	const double GetDeltaX() const
	{
		return m_dx;
	}
	JFigure& SetDeltaX(double dx)
	{
		m_dx = dx;
		return *this;
	}

	const double GetDeltaY() const
	{
		return m_dy;
	}
	JFigure& SetDeltaY(double dy)
	{
		m_dy = dy;
		return *this;
	}

	const double GetDeltaZ() const
	{
		return m_dz;
	}
	JFigure& SetDeltaZ(double dz)
	{
		m_dz = dz;
		return *this;
	}

	
	JFigure& DeserializeFromDocument(const Value& jsonDocument, LPLogMsgHandler messageHandler)
	{
		AssignValue(jsonDocument, string(JF_KEY_ID), m_id, messageHandler);

		AssignValue(jsonDocument, string(JF_KEY_X), m_x, messageHandler);
		AssignValue(jsonDocument, string(JF_KEY_Y), m_y, messageHandler);
		AssignValue(jsonDocument, string(JF_KEY_Z), m_z, messageHandler);

		AssignValue(jsonDocument, string(JF_KEY_dX), m_dx, messageHandler);
		AssignValue(jsonDocument, string(JF_KEY_dY), m_dy, messageHandler);
		AssignValue(jsonDocument, string(JF_KEY_dZ), m_dz, messageHandler);

		messageHandler(ToString(), false);

		return *this;
	}

	string ToString() const
	{
		stringstream ss;
		ss << JF_KEY_ID << ":" << m_id
			<< ";" << JF_KEY_X << ":" << m_x
			<< ";" << JF_KEY_Y << ":" << m_y;

		if (m_z > 0 || m_dz>0)
		{
			ss << ";" << JF_KEY_Z << ":" << m_z;
		}

		if (m_dx>0 || m_dy>0 || m_dz>0)
		{
			ss << ";" << JF_KEY_dX << ":" << m_dx;
		}

		if (m_dy>0 || m_dz>0)
		{
			ss << ";" << JF_KEY_dY << ":" << m_dy;
		}

		if (m_dz>0)
		{
			ss << ";" << JF_KEY_dZ << ":" << m_dz;
		}

		ss << std::endl;

		return ss.str();
	}

	JFigure& DeserializeFromTransfer(const char* pJReceived, LPLogMsgHandler messageHandler)
	{
		if (!pJReceived || strlen(pJReceived)==0)
		{
			messageHandler("Deserialize from transfer terminated\nCAUSE: Bad (empty) json script.", true);
		}
		else
		{
			string json(pJReceived);
			Document document;
			stringstream ssMsg;

			if (document.Parse(json.c_str()).HasParseError())
			{				
				ssMsg << "The '" << json << "' text is failed to parse as json. " << "ERROR: " << document.GetParseError();
				messageHandler(ssMsg.str(), true);
			}
			//else if (document.MemberBegin() == document.MemberEnd())
			//{ }
			else
			{
				if (document.IsObject())
					DeserializeFromDocument(document.GetObjectA(), messageHandler);
				else
				{
					ssMsg << "The '" << json << "' text is failed to parse as json.";
					messageHandler(ssMsg.str(), true);
				}
			}
		}
		return *this;
	}
	
	string SerializeToTransfer() const
	{
		StringBuffer sb;
		PrettyWriter<StringBuffer> writer(sb);

		Serialize(writer);

		return sb.GetString();
	}
#pragma endregion | Methods |

};


class JFigures
{
#pragma region | Fields |
private:
	string          m_jsonFileName;
	Document        m_documentFromFile;
	vector<JFigure> m_collection;
	int             m_expectedQty;
protected:
public:

#pragma endregion | Fields |

#pragma region | Constructors |

private:

protected:

public:
	JFigures()
		: m_jsonFileName()
		, m_collection()
		, m_documentFromFile()
		, m_expectedQty(0)
	{
	}

	virtual ~JFigures()
	{
	}

#pragma endregion | Constructors |

#pragma region | Methods |
private:
	struct JFigurePredicateById
	{
	private:
		string m_id;

	public:
		JFigurePredicateById(const string id)
			: m_id(id)
		{}

		bool operator()(const JFigure& jf)
		{
			return jf.GetId().compare(m_id) == 0;
		}
	};
	
protected:
	template <typename Writer>
	void Serialize(Writer& writer) const {

		// This base class just write out name-value pairs, without wrapping within an object.
		writer.String("figures");

		writer.StartArray();

		vector<JFigure>::const_iterator ci = m_collection.begin();
		for (; ci != m_collection.end(); ++ci)
		{
			ci->Serialize(writer);
		}

		writer.EndArray();
	}

public:
	const string& GetJsonFileName() const
	{
		return m_jsonFileName;
	}

	const vector<JFigure>& GetFigures() const
	{
		return m_collection;
	}

	bool IsValid() const
	{
		return !m_collection.empty() && m_expectedQty==m_collection.size();
	}

	const JFigure* GetFigure(const string& id) const
	{
		const JFigure* pRes = NULL;
		JFigurePredicateById predicate(id);

		vector<JFigure>::const_iterator ci = std::find_if(m_collection.begin(), m_collection.end(), predicate);

		if (ci != m_collection.end())
			pRes = &*ci;

		return pRes;
	}
	JFigures& Add(const JFigure& jf)
	{
		const JFigure* pFig = GetFigure(jf.GetId());

		if (!pFig)
			m_collection.push_back(jf);
		else
			(*(JFigure*)pFig) = jf;

		return *this;
	}

	JFigures& DeserializeFromFile(const string& jsonFileName, LPLogMsgHandler messageHandler)
	{
		stringstream ssMsg;

		if (jsonFileName.empty())
		{
			messageHandler( "Deserialize from file terminated\nCAUSE: Bad (empty) script file name.", true);
		}
		else if (jsonFileName.find(".json") <= 0)
		{
			ssMsg << "The '" << jsonFileName << "' file is not a json.";
			messageHandler(ssMsg.str(), true);
		}
		else
		{
			// Temporary container for the file content
			stringstream ss;

			// Open file for reading
			ifstream i(jsonFileName, ios::binary);
			ss << i.rdbuf();
			i.close();

			//if (!m_documentFromFile.Empty())
			//{
			//	JSN_SENDINFO_1("Json object has been clean before loading a new '%s' file.", jFileName);
			//}

			if (m_documentFromFile.Parse<0>(ss.str().c_str()).HasParseError())
			{
				ssMsg << "The '" << jsonFileName << "' file is failed to read. " << "ERROR: " << m_documentFromFile.GetParseError();
				messageHandler(ssMsg.str(), true);
			}
			else
			{
				bool isError = false;
				m_collection.clear();

				DeserializeFromDocument(m_documentFromFile, messageHandler);

				ssMsg << "The '" << jsonFileName << "' json file is ";
				if (m_collection.size() == m_expectedQty)
				{
					m_jsonFileName = jsonFileName;
					ssMsg << "successfully loaded (actual quantity '" << m_collection.size() << "' == expected '" << m_expectedQty << "').";
				}
				else
				{
					isError = true;
					ssMsg << "failed to load (actual quantity '" << m_collection.size() << "' != expected '" << m_expectedQty << "').";
				}
				messageHandler(ssMsg.str(), isError);
			}
		}
		
		return *this;
	}

	JFigures& DeserializeFromDocument(const Document& jsonDocument, LPLogMsgHandler messageHandler)
	{
		Value::ConstMemberIterator iter = jsonDocument.MemberBegin();
		for (; iter != jsonDocument.MemberEnd(); ++iter)
		{
			if (iter->value.GetType() == Type::kArrayType)
			{
				Value::ConstArray jfsArray = iter->value.GetArray();
				m_expectedQty = jfsArray.Size();

				for (Value::ConstValueIterator jfs_ci = jfsArray.Begin(); jfs_ci != jfsArray.End(); ++jfs_ci)
				{
					m_collection.push_back(JFigure("").DeserializeFromDocument((*jfs_ci), messageHandler));
				}
			}
			else if (iter->value.GetType() == Type::kObjectType)
			{
				
			}
		}

		return *this;
	}

	JFigures& SerializeToFile(const string& jsonFileName, LPLogMsgHandler messageHandler)
	{
		if (jsonFileName.empty())
			((string)jsonFileName) = m_jsonFileName;

		if (jsonFileName.empty())
		{

		}
		else if (m_collection.empty())
		{

		}
		else
		{
			StringBuffer sb;
			PrettyWriter<StringBuffer> writer(sb);

			writer.StartObject();
			Serialize(writer);
			writer.EndObject();

			string all = sb.GetString();

			std::ofstream o(jsonFileName);
			o << all << std::endl;
			o.close();
		}

		return *this;
	}


#pragma endregion | Methods |

};