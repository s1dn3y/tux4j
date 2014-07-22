#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#pragma comment(lib, "libfml.lib")
#pragma comment(lib, "libfml32.lib")

#include <iostream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <stdexcept>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::list;
using std::copy;
using std::back_inserter;
using std::transform;
using std::out_of_range;
using std::ostringstream;
using std::istringstream;

#include <atmi.h>

#include "exceptions"
#include "fieldbuf.h"

/*
FML16FieldBuffer::

	void addField(const char * fldName, short fldValue);
	void addField(const char * fldName, long fldValue);
	void addField(const char * fldName, char fldValue);
	void addField(const char * fldName, float fldValue);
	void addField(const char * fldName, double fldValue);
	void addField(const char * fldName, const char * fldValue); // string
	void addField(const char * fldName, const char * fldValue, FldlenType fldLength); // carray
	short getFieldValue(const char * fldName);
	long getFieldValue(const char * fldName);
	char getFieldValue(const char * fldName);
	float getFieldValue(const char * fldName);
	double getFieldValue(const char * fldName);
	char * getFieldValue(const char * fldName); // string ou carray
	FldlenType getFieldLength(const char * fldName); // carray
*/

const FML16FieldBuffer::FldidType FML16FieldBuffer::_BADFLDID = (FldidType) 0;


FML16FieldBuffer::FML16FieldBuffer()
{
	FLDTBLDIR_ENVVAR = "FLDTBLDIR";
	FIELDTBLS_ENVVAR = "FIELDTBLS";

	setType("FML");
	setSubtype("");
}

void FML16FieldBuffer::addField(const char * fldName, const char * fldValue, FldlenType fldLen)
{
	int fldType;
	FldidType fldId = fml16::Fldid(const_cast<char *>(fldName));

	if (fldId == _BADFLDID) {
		ostringstream fmt;
		fmt << fldName << " não é um nome de campo válido";
		throw BufferHandlingException(fmt.str());
	}

	fldType = fml16::Fldtype(fldId);

	if (fldType == FLD_CARRAY && fldLen < 1) {
		ostringstream fmt;
		fmt << "O tamanho do campo " << fldName << " e invalido";
		throw BufferHandlingException(fmt.str());
	}

	switch (fldType) {
		case FLD_SHORT:		fldLen = sizeof(short);		break;
		case FLD_LONG:		fldLen = sizeof(long);		break;
		case FLD_CHAR:		fldLen = sizeof(char);		break;
		case FLD_FLOAT:		fldLen = sizeof(float);		break;
		case FLD_DOUBLE:	fldLen = sizeof(double);	break;
		case FLD_STRING:	fldLen = strlen(fldValue);	break;
	}

	if (fields.find(fldId) != fields.end()) {
		fields[fldId].push_back(FieldAttr(fldValue, fldLen));
	} else {
		FieldAttrListType fldList;
		fldList.push_back(FieldAttr(fldValue, fldLen));
		fields[fldId] = fldList;
	}
}

/*
void FML16FieldBuffer::addField(const char * fldName, const char * fldValue)
{
	int fldType;
	FldidType fldId = fml16::Fldid(fldName);

	if (fldId == fml16::BADFLDID) {
		ostringstream fmt;
		fmt << fldName << " não é um nome de campo válido";
		throw BufferHandlingException(fmt.str());
	}

	if (fields.find(fldId) != fields.end()) {
		fields[fldId].push_back(FieldAttr<short>(fldVal, sizeof(short)));
	} else {
		FieldAttrListType fldList;
		fldList.push_back(FieldAttr<short>(fldVal, sizeof(short)));
		fields[fldId] = fldList;
	}
}
*/