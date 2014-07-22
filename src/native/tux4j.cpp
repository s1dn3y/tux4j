// wraptux.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include <atmi.h>
#include <iostream>
#include <string>
// #include <ios>


#include "tux4j.h"
#include "cligentuxf.h"

#pragma comment(lib, "libgp.lib")
#pragma comment(lib, "libwsc.lib") // OU: #pragma comment(lib, "libtux.lib")

using std::string;
using std::cout;

GenericTuxedoClient client;

jclass findClass(JNIEnv *env, string classFullName)
{
	jclass result;
	result = env->FindClass(classFullName.c_str());
	if (result == 0) cout << "Erro: classe não encontrada ("<< classFullName <<")\n";
	return result;
}


JNIEXPORT void JNICALL Java_com_ezi_tux4j_GenericClient_tuxConnect(
														   JNIEnv *env,
														   jobject obj,
														   jstring sAddr,
														   jstring sPort)
{
	jboolean iscopy;
	jclass jvmExceptionClass;
	jclass cls = env->GetObjectClass(obj);
	jfieldID fid;
	jstring jstr;
	const char *tmp;

	tmp = env->GetStringUTFChars(sAddr, &iscopy);
	string srvAddr = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(sAddr, tmp);

	tmp = env->GetStringUTFChars(sPort, &iscopy);
	string srvPort = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(sPort, tmp);

	fid = env->GetFieldID(cls, "tuxedoClientDir", "Ljava/lang/String;");
	jstr = (jstring) env->GetObjectField(obj, fid);
	tmp = env->GetStringUTFChars(jstr, &iscopy);
	string tuxedoCltientDir = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(jstr, tmp);

	if (!tuxedoCltientDir.empty())
		GenericTuxedoClient::checkEnvVar("TUXDIR", tuxedoCltientDir);

	fid = env->GetFieldID(cls, "fmlTableDir", "Ljava/lang/String;");
	jstr = (jstring) env->GetObjectField(obj, fid);
	tmp = env->GetStringUTFChars(jstr, &iscopy);
	string fmlTableDir = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(jstr, tmp);

	if (!fmlTableDir.empty())
		GenericTuxedoClient::checkEnvVar(FLDTBLDIR_1632, fmlTableDir);

	fid = env->GetFieldID(cls, "fmlTables", "Ljava/lang/String;");
	jstr = (jstring) env->GetObjectField(obj, fid);
	tmp = env->GetStringUTFChars(jstr, &iscopy);
	string fmlTables = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(jstr, tmp);

	if (!fmlTables.empty())
		GenericTuxedoClient::checkEnvVar(FIELDTBLS_1632, fmlTables);

	try {
		client.connect(srvAddr, srvPort);
	} catch (TPConnectionException ex) {

		jvmExceptionClass = findClass(env, "com/ezi/tux4j/TPConnectionException");
		if (jvmExceptionClass != 0) env->ThrowNew(jvmExceptionClass, ex.what());

	}
}

JNIEXPORT void JNICALL Java_com_ezi_tux4j_GenericClient_tuxDisconnect(JNIEnv *env, jobject obj)
{
	client.disconnect();
}

JNIEXPORT void JNICALL Java_com_ezi_tux4j_GenericClient_bufAddField(JNIEnv *env,
																		  jobject obj,
																		  jstring fname,
																		  jlong flen,
																		  jstring fval,
																		  jint fmod)
{
	jboolean iscopy;
	jclass jvmExceptionClass;
	const char *tmp;

	tmp = env->GetStringUTFChars(fname, &iscopy);
	string fldName = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(fname, tmp);

	tmp = env->GetStringUTFChars(fval, &iscopy);
	string fldValue = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(fval, tmp);

	try {
		client.addSvcParm(fldName, (FLDLEN_1632) flen, fldValue, fmod);
	} catch (BufferHandlingException ex) {

		jvmExceptionClass = findClass(env, "com/ezi/tux4j/BufferHandlingException");
		// se achou a classe gera a excessão
		if (jvmExceptionClass != 0)
			env->ThrowNew(jvmExceptionClass, ex.what());

	}
}

JNIEXPORT jstring JNICALL Java_com_ezi_tux4j_GenericClient_bufGetFieldValue(
																	JNIEnv *env,
																	jobject obj,
																	jstring fname,
																	jlong focc)
{
	jboolean iscopy;
	jclass jvmExceptionClass;
	const char *tmp;
	string fldName;

	if (fname != NULL) {
		tmp = env->GetStringUTFChars(fname, &iscopy);
		fldName = const_cast<char *>(tmp);
		env->ReleaseStringUTFChars(fname, tmp);
	}

	string fldValue;

	try {
		fldValue = client.getSvcParmValue(fldName, focc);
	} catch(BufferHandlingException ex) {

		jvmExceptionClass = findClass(env, "com/ezi/tux4j/BufferHandlingException");
		// se achou a classe gera a excessão
		if (jvmExceptionClass != 0)
			env->ThrowNew(jvmExceptionClass, ex.what());

	}

	return env->NewStringUTF(fldValue.c_str());
}



JNIEXPORT void JNICALL Java_com_ezi_tux4j_GenericClient_tuxRun(JNIEnv *env,
																	 jobject obj,
																	 jstring sname)
{
	jclass jvmExceptionClass;
	jboolean iscopy;
	const char *tmp;

	tmp = env->GetStringUTFChars(sname, &iscopy);
	string svcName = const_cast<char *>(tmp);
	env->ReleaseStringUTFChars(sname, tmp);

	try {
		client.run(svcName);
	} catch (TPFatalException ex) {

		jvmExceptionClass = findClass(env, "com/ezi/tux4j/TPFatalException");
		if (jvmExceptionClass != 0) env->ThrowNew(jvmExceptionClass, ex.what());

	} catch (BufferHandlingException ex) {

		jvmExceptionClass = findClass(env, "com/ezi/tux4j/BufferHandlingException");
		if (jvmExceptionClass != 0) env->ThrowNew(jvmExceptionClass, ex.what());

	}
}
