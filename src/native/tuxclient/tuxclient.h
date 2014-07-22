#define VERSION "0.1b"
#pragma comment(lib, "libgp.lib")
#pragma comment(lib, "libwsc.lib") // OU: #pragma comment(lib, "libtux.lib")

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

// stl
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

// tuxedo
#include <atmi.h>
#include <Uunix.h>
//#include <Usignal.h>
#ifndef __WITH_FML32
	#include <fml.h>
	#define FML_RELEASE_INFO "Compilada com suporte a FML16"
	#define FMLTYPE_1632 "FML"
	#define FLDTBLDIR_1632 "FLDTBLDIR"
	#define FIELDTBLS_1632 "FIELDTBLS"
	#define FLDID_1632 FLDID
	#define FLDLEN_1632 FLDLEN
	#define FLDOCC_1632 FLDOCC
	#define FBFR_1632 FBFR
	#define Fldid_1632 Fldid
	#define Fldtype_1632 Fldtype
	#define Fname_1632 Fname
	#define Fneeded_1632 Fneeded
	#define Fdelall_1632 Fdelall
	#define Fielded_1632 Fielded
	#define Fappend_1632 Fappend
	#define Findex_1632 Findex
	#define Fprint_1632 Fprint
	#define Foccur_1632 Foccur
	#define Ffind_1632 Ffind
	#pragma comment(lib, "libfml.lib")
#else
	#include <fml32.h>
	#define FML_RELEASE_INFO "Compilada com suporte a FML32"
	#define FMLTYPE_1632 "FML32"
	#define FLDTBLDIR_1632 "FLDTBLDIR32"
	#define FIELDTBLS_1632 "FIELDTBLS32"
	#define FLDID_1632 FLDID32
	#define FLDLEN_1632 FLDLEN32
	#define FLDOCC_1632 FLDOCC32
	#define FBFR_1632 FBFR32
	#define Fldid_1632 Fldid32
	#define Fldtype_1632 Fldtype32
	#define Fname_1632 Fname32
	#define Fneeded_1632 Fneeded32
	#define Fdelall_1632 Fdelall32
	#define Fielded_1632 Fielded32
	#define Fappend_1632 Fappend32
	#define Findex_1632 Findex32
	#define Fprint_1632 Fprint32
	#define Foccur_1632 Foccur32
	#define Ffind_1632 Ffind32
	#pragma comment(lib, "libfml32.lib")

//
//	typedef unsigned long (__stdcall * Fldid_Ptr) (char *);

#endif

//#include "util.h"

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

#include "exceptions"
#include "fieldbuf.h"

class FieldAttr
{
private:
	FLDLEN_1632 length;
	string value;
public:
	FieldAttr(FLDLEN_1632 length = 0, string value = "");
	FLDLEN_1632 getLength(void) {return length;}
	string getValue(void) { return value;}
};

typedef vector<FieldAttr> FieldAttrVector;
typedef map<FLDID_1632, FieldAttrVector> FieldRepMap;
typedef unsigned char FieldMode;
//typedef void (*signalHdlrFuncPtr)(int);
//typedef map<int,signalHdlrFuncPtr> SignalHandlersMap;

class GenericTuxedoClient
{
private:
	static const FieldMode PARM_IN;
	static const FieldMode PARM_OUT;
	static const FieldMode PARM_INOUT;

	bool batchMode, endOfBatch, connected, quietMode;
	FLDID_1632 batchCtrlField;
	FBFR_1632 *iFieldBuffer, *oFieldBuffer;
	string valueToStopBatch;
	string outFilename;
	FieldRepMap inFieldsMap, outFieldsMap;
	int maxConsecFails;
//	SignalHandlersMap oldSignalHandlers;
//	void addSvcParm(string name, FLDLEN_1632 length, string value, char mode);
	long getBuffNeededSpace(FieldMode fm);
	void allocFieldBuffer(FieldMode fm);
	void deleteFieldBuffer(FieldMode fm);
	void clearFieldBuffer(FieldMode fm);
	void populateInputFieldBuffer(void);
	void retrieveOutputFields(void);
	void recycleOutputFields(void);
	void showFields(FieldMode fm);

public:
	GenericTuxedoClient(void);
	~GenericTuxedoClient(void);
	void addSvcParm(string name, FLDLEN_1632 length, string value, char mode);
	void addSvcParmIN(string name, FLDLEN_1632 length, string value);
	void addSvcParmOUT(string name, FLDLEN_1632 length, string value);
	void addSvcParmINOUT(string name, FLDLEN_1632 length, string value);
	string getSvcParmValue(string fldName, FLDOCC_1632 fldOcc)
		throw (BufferHandlingException);
	const FieldRepMap& getInFields(void);
	const FieldRepMap& getOutFields(void);

	bool isInBatchMode(void);
	void setBatchMode(bool isBatch);
	void setBatchCtrlField(string batchCF);
	void setValueToStopBatch(string value);
	void setOutput(string ofn);
	void setInputFields(FieldRepMap inFldMap);
	void setOutputFields(FieldRepMap outFldMap);
	static void checkEnvVar(string envKey, string envValue);
	void setQuietMode(bool beQuiet);
	bool isInQuietMode(void);
	void setMaxConsecutiveFails(int value);
	int getMaxConsecutiveFails(void);
//	void addSignalHandler(int signal, signalHdlrFuncPtr handler);
	void restoreSignalHandlers(void) throw (SignalHandlingException);
	void connect(const string &srvAddr, const string &srvPort) /* throw(...)*/ ;
	void disconnect(void) /* throw(...)*/ ;
	void run(const string &svcName) /* throw(...)*/;
};
