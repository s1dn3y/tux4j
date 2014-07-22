class FieldBuffer
{
private:
	string type;
	string subtype;
public:
	virtual ~FieldBuffer();
	inline void setType(const char * value) {type.assign(value);}
	inline void setSubtype(const char * value) {subtype.assign(value);}
	inline static void checkEnvVar(string envKey, string envValue) throw (Exception)
	{
		if (! envKey.empty() && ! envValue.empty()) {
			string ENV = envKey + "=" + envValue;
			if (tuxputenv(const_cast<char *>(ENV.c_str())) < 0)
				throw Exception("Falha em tuxputenv(\"" + ENV + "\")");
		}

		if (tuxgetenv(const_cast<char *>(envKey.c_str())) == NULL)
			throw Exception("A variavel de ambiente " + envKey + " nao esta presente");
	}
};

class FMLFieldBuffer : public FieldBuffer
{
private:
protected:
	const char * FLDTBLDIR_ENVVAR;
	const char * FIELDTBLS_ENVVAR;
public:
	virtual ~FMLFieldBuffer();
	inline void setFieldTableDir(const char * value){checkEnvVar(FLDTBLDIR_ENVVAR, value);}
	inline void setFieldTables(const char * value){checkEnvVar(FIELDTBLS_ENVVAR, value);}
};

namespace fml16
{
#include "fml.h"
}


class FML16FieldBuffer : public FMLFieldBuffer
{
private:
	typedef fml16::FBFR FbfrType;
	typedef fml16::FLDID FldidType;
	typedef fml16::FLDLEN FldlenType;
	typedef fml16::FLDOCC FldoccType;

	/*
	fml16::Fldid()
	fml16::Fldtype()
	fml16::Fname()
	fml16::Fneeded()
	fml16::Fdelall()
	fml16::Fielded()
	fml16::Fappend()
	fml16::Findex()
	fml16::Fprint()
	fml16::Foccur()
	fml16::Ffind()
	*/

	class FieldAttr
	{
	private:
		string value;
		FldlenType length;
	public:
		FieldAttr(string value, FldlenType length = 0);
		inline string getValue(void) {return value;}
		inline FldlenType getLength(void) {return length;}
	};

	typedef vector<FieldAttr> FieldAttrListType;
	typedef map<FldidType, FieldAttrListType> FieldsMap;

	FieldsMap fields;
	FbfrType *buffer;

	static const FldidType _BADFLDID;

public:
	FML16FieldBuffer();
	void addField(const char * fldName, const char * fldValue, FldlenType fldLen = 0);
=>	string getFieldValue(const char * fldName);
=>	FldlenType getFieldLength(const char * fldName); // carray
//	FbfrType* getBuffer(); // chama Fneeded(), aloca o buffer, popula-o e retorna-o.
//	void setBuffer(FbfrType* value); // limpa "fields", navega pelo buffer e popula "fields"
};


//namespace fml32{
//#include "fml32.h"
//}

class FML32FieldBuffer : public FMLFieldBuffer
{
private:
public:
};