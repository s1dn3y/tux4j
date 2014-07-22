#include "tuxclient.h"

const FieldMode GenericTuxedoClient::PARM_IN = 1;
const FieldMode GenericTuxedoClient::PARM_OUT = 2;
const FieldMode GenericTuxedoClient::PARM_INOUT = 3;

bool GenericTuxedoClient::isInBatchMode(void)
{
	return batchMode;
}

void GenericTuxedoClient::setBatchMode(bool isBatch)
{
	batchMode = isBatch;
}

void GenericTuxedoClient::setBatchCtrlField(string batchCF)
{
	batchCtrlField = Fldid_1632(const_cast<char *>(batchCF.c_str()));
}

void GenericTuxedoClient::setValueToStopBatch(string value)
{
	valueToStopBatch = value;
}

void GenericTuxedoClient::setOutput(string ofn)
{
	outFilename = ofn;
}

void GenericTuxedoClient::setQuietMode(bool beQuiet)
{
	quietMode = beQuiet;
}

bool GenericTuxedoClient::isInQuietMode(void)
{
	return quietMode;
}

void GenericTuxedoClient::setMaxConsecutiveFails(int value)
{
	maxConsecFails = value;
}

int GenericTuxedoClient::getMaxConsecutiveFails(void)
{
	return maxConsecFails;
}

/*
void GenericTuxedoClient::addSignalHandler(int signal, signalHdlrFuncPtr handler)
{
	if (Usignal(signal, handler) == SIG_ERR) {
		ostringstream fmt;
		fmt << "incluindo tratador do sinal " << signal;
		throw SignalHandlingException(fmt.str(), __FILE__, __LINE__);
	}
	oldSignalHandlers[signal] = handler;
}
*/

/*
void GenericTuxedoClient::restoreSignalHandlers(void) throw (SignalHandlingException)
{
	SignalHandlersMap::iterator it;
	for (it = oldSignalHandlers.begin(); it != oldSignalHandlers.end(); it++)
		if (Usignal(it->first, it->second) == SIG_ERR) {
			ostringstream fmt;
			fmt << "restaurando tratador do sinal" << it->first;
			throw SignalHandlingException(fmt.str());
		}
	oldSignalHandlers.clear();
}
*/

FieldAttr::FieldAttr(FLDLEN_1632 length, string value)
{
	this->length = length;
	if (! value.empty()) this->value = value;
}

GenericTuxedoClient::GenericTuxedoClient(void)
{
	batchMode = false;
	endOfBatch = false;
	connected = false;
	quietMode = false;
	maxConsecFails = 0;
	iFieldBuffer = NULL;
	oFieldBuffer = NULL;
}

GenericTuxedoClient::~GenericTuxedoClient(void)/* throw (E...FieldBuffer) */
{
	if (connected) disconnect();
	inFieldsMap.clear();
	outFieldsMap.clear();
}

void GenericTuxedoClient::addSvcParm(string fldName, FLDLEN_1632 fldLen,
									 string fldVal, char mode)
throw (BufferHandlingException)
{
	if (mode != PARM_IN && mode != PARM_OUT && mode != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	int fldType;
	FLDID_1632 fldId = Fldid_1632(const_cast<char *>(fldName.c_str()));

	if (fldId == BADFLDID) {
		ostringstream fmt;
		fmt << fldName << " não é um nome de campo válido";
		throw BufferHandlingException(fmt.str());
	}

	fldType = Fldtype_1632(fldId);

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
		case FLD_STRING:	fldLen = fldVal.length();	break;
	}

	if ((mode & PARM_IN) == PARM_IN) {
		if (inFieldsMap.find(fldId) != inFieldsMap.end()) {
			inFieldsMap[fldId].push_back(FieldAttr(fldLen, fldVal));
		} else {
			FieldAttrVector fldVec;
			fldVec.push_back(FieldAttr(fldLen, fldVal));
			inFieldsMap[fldId] = fldVec;
		}
	}

	if ((mode & PARM_OUT) == PARM_OUT) {
		if (mode != PARM_INOUT && ! fldVal.empty())
			cout << "Ignorando o valor inicial do campo OUT " << Fname_1632(fldId)
				 << (" << fldVal << ") << endl;

		if (outFieldsMap.find(fldId) != outFieldsMap.end()) {
			outFieldsMap[fldId].push_back(FieldAttr(fldLen, ""));
		} else {
			FieldAttrVector fldVec;
			fldVec.push_back(FieldAttr(fldLen, ""));
			outFieldsMap[fldId] = fldVec;
		}
	}
}

long GenericTuxedoClient::getBuffNeededSpace(FieldMode fm)
{
	if (fm != PARM_IN && fm != PARM_OUT && fm != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	FLDOCC_1632 fldCount = 0;
	FLDLEN_1632 fldsLength = 0;

	if ((fm & PARM_IN) == PARM_IN) {
		for (FieldRepMap::iterator it1 = inFieldsMap.begin();
			 it1 != inFieldsMap.end(); it1++)
		{
			int fldType = Fldtype_1632(it1->first);

			for (FieldAttrVector::iterator it2 = it1->second.begin();
				 it2 != it1->second.end(); it2++)
			{
				fldCount++;
				fldsLength += it2->getLength();
			}
		}
	}

	if ((fm & PARM_IN) == PARM_OUT) {
		for (FieldRepMap::iterator it1 = outFieldsMap.begin();
			 it1 != outFieldsMap.end(); it1++)
		{
			int fldType = Fldtype_1632(it1->first);

			for (FieldAttrVector::iterator it2 = it1->second.begin();
				 it2 != it1->second.end(); it2++)
			{
				fldCount++;
				fldsLength += it2->getLength();
			}
		}
	}

	return(Fneeded_1632(fldCount, fldsLength));
}

void GenericTuxedoClient::allocFieldBuffer(FieldMode fm) throw (BufferHandlingException)
{
	if (fm != PARM_IN && fm != PARM_OUT && fm != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	if ((fm & PARM_IN) == PARM_IN) {
		iFieldBuffer = (FBFR_1632 *) tpalloc(FMLTYPE_1632, NULL, getBuffNeededSpace(PARM_IN));
		if (iFieldBuffer == NULL)
			throw BufferHandlingException(tpstrerror(tperrno));
	}

	if ((fm & PARM_OUT) == PARM_OUT) {
		oFieldBuffer = (FBFR_1632 *) tpalloc(FMLTYPE_1632, NULL, getBuffNeededSpace(PARM_OUT));
		if (oFieldBuffer == NULL)
			throw BufferHandlingException(tpstrerror(tperrno));
	}
}

void GenericTuxedoClient::deleteFieldBuffer(FieldMode fm)
{
	if (fm != PARM_IN && fm != PARM_OUT && fm != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	if ((fm & PARM_IN) == PARM_IN) {
		tpfree(reinterpret_cast<char *>(iFieldBuffer));
	}

	if ((fm & PARM_OUT) == PARM_OUT) {
		tpfree(reinterpret_cast<char *>(oFieldBuffer));
	}
}

void GenericTuxedoClient::clearFieldBuffer(FieldMode fm) throw (BufferHandlingException)
{
	if (fm != PARM_IN && fm != PARM_OUT && fm != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	if ((fm & PARM_IN) == PARM_IN) {
		for (FieldRepMap::iterator it1 = inFieldsMap.begin();
			 it1 != inFieldsMap.end(); it1++)
		{
			Fdelall_1632(iFieldBuffer, it1->first);
		}
	}

	if ((fm & PARM_OUT) == PARM_OUT) {
		for (FieldRepMap::iterator it1 = outFieldsMap.begin();
			 it1 != outFieldsMap.end(); it1++)
		{
			Fdelall_1632(oFieldBuffer, it1->first);
		}
	}
}

void GenericTuxedoClient::populateInputFieldBuffer(void)
throw(BufferNotFieldedException,BufferHandlingException,InvalidNumberFormatException)
{
	if (! Fielded_1632(iFieldBuffer)) {
		throw BufferNotFieldedException("Buffer FML nao inicializado");
	}

	clearFieldBuffer(PARM_IN);

	for (FieldRepMap::iterator it1 = inFieldsMap.begin();
		 it1 != inFieldsMap.end(); it1++)
	{
		// Popula o buffer com as ocorrências do campo atual
		for (FieldAttrVector::iterator it2 = it1->second.begin();
			 it2 != it1->second.end(); it2++)
		{
			int result;
			istringstream fmt;

			fmt.str(it2->getValue());

			switch(Fldtype_1632(it1->first)) {

			case FLD_SHORT:
			{
				short value;
				try {
					fmt >> value;
				} catch(...) {
					throw InvalidNumberFormatException("Valor invalido para o tipo short: " + fmt.str());
				}
				result = Fappend_1632(iFieldBuffer, it1->first,
								 reinterpret_cast<char *>(&value),
								 (FLDLEN_1632) 0);
				break;
			}

			case FLD_LONG:
			{
				long value;
				try {
					fmt >> value;
				} catch(...) {
					throw InvalidNumberFormatException("Valor invalido para o tipo long: " + fmt.str());
				}
				result = Fappend_1632(iFieldBuffer, it1->first,
								 reinterpret_cast<char *>(&value),
								 (FLDLEN_1632) 0);
				break;
			}

			case FLD_CHAR:
			{
				char value;
				try {
					fmt >> value;
				} catch(...) {
					throw InvalidNumberFormatException("Valor invalido para o tipo char: " + fmt.str());
				}
				result = Fappend_1632(iFieldBuffer, it1->first,
								 reinterpret_cast<char *>(&value),
								 (FLDLEN_1632) 0);
				break;
			}

			case FLD_FLOAT:
			{
				float value;
				try {
					fmt >> value;
				} catch(...) {
					throw InvalidNumberFormatException("Valor invalido para o tipo float: " + fmt.str());
				}
				result = Fappend_1632(iFieldBuffer, it1->first,
								 reinterpret_cast<char *>(&value),
								 (FLDLEN_1632) 0);
				break;
			}

			case FLD_DOUBLE:
			{
				double value;
				try {
					fmt >> value;
				} catch(...) {
					throw InvalidNumberFormatException("Valor invalido para o tipo double: " + fmt.str());
				}
				result = Fappend_1632(iFieldBuffer, it1->first,
								 reinterpret_cast<char *>(&value),
								 (FLDLEN_1632) 0);
				break;
			}

			case FLD_STRING:

				result = Fappend_1632(iFieldBuffer, it1->first,
								 const_cast<char *>(it2->getValue().c_str()),
								 (FLDLEN_1632) 0);
				break;

			case FLD_CARRAY:

				result = Fappend_1632(iFieldBuffer, it1->first,
								 const_cast<char *>(it2->getValue().c_str()),
								 (FLDLEN_1632) it2->getValue().size());
				break;

			}

			if (result < 0)
				throw BufferHandlingException(tpstrerror(tperrno));

		}
		Findex_1632(iFieldBuffer, 0);
	}
}

void GenericTuxedoClient::retrieveOutputFields(void) throw (BufferHandlingException)
{
	FLDID_1632  fldId;
	FLDOCC_1632 fldOcc;
	FLDLEN_1632 fldLen;

/*
		cout << "oFieldBuffer possui " << Fnum(oFieldBuffer) << " campos\n";

		FLDID fieldid;
		FLDOCC occurrence;
		char val[1000];
		FLDLEN len;

		for( fieldid=FIRSTFLDID, len=sizeof(val);
			 Fnext(oFieldBuffer, &fieldid, &occurrence, val, &len) > 0;
			 len=sizeof(val))
		{
			cout << Fname(fieldid) << "(" << fieldid << ":" << occurrence << ") = [";
			cout << string(val,len) << "]\n";
		}
*/

#ifdef _DEBUG
		cout << "\n\nDISPLAY em retrieveOutputFields: INICIO\n";
		Fprint_1632(oFieldBuffer);
		cout << "DISPLAY em retrieveOutputFields: FINAL\n";
#endif

	for (FieldRepMap::iterator it1 = outFieldsMap.begin();
		 it1 != outFieldsMap.end(); it1++)
	{
		// força a limpeza da lista de valores do campo
		fldId = it1->first;
//		it1->second.erase(it1->second.begin(),it1->second.end());
		it1->second.clear();
		fldOcc = Foccur_1632(oFieldBuffer, fldId);

		if (fldOcc < 0) throw BufferHandlingException(tpstrerror(tperrno));
		if (fldOcc == 0) continue;

		for (FLDOCC_1632 occ = 0; occ < fldOcc; occ++) {

			ostringstream fmt;
			char *value;

			if((value = Ffind_1632(oFieldBuffer, fldId, occ, &fldLen)) == NULL)
				throw BufferHandlingException(tpstrerror(tperrno));

			switch(Fldtype_1632(fldId)) {
			case FLD_SHORT:

				fmt << reinterpret_cast<short *>(value);
				break;

			case FLD_LONG:

				fmt << reinterpret_cast<long *>(value);
				break;

			case FLD_CHAR:

				fmt << reinterpret_cast<char *>(value);
				break;

			case FLD_FLOAT:

				fmt << reinterpret_cast<float *>(value);
				break;

			case FLD_DOUBLE:

				fmt << reinterpret_cast<double *>(value);
				break;

			case FLD_STRING:
			case FLD_CARRAY:

				fmt << string(value, fldLen);
				break;

			}

			it1->second.push_back(FieldAttr(fldLen, fmt.str()));

			if (batchMode && it1->first == batchCtrlField && fmt.str() == valueToStopBatch)
				endOfBatch = true;
		}
	}
}

void GenericTuxedoClient::showFields(FieldMode fm)
throw (BufferHandlingException)
{
	if (fm != PARM_IN && fm != PARM_OUT && fm != PARM_INOUT) {
		throw BufferHandlingException(
			"O modo do parametro deve ser PARM_IN, PARM_OUT ou PARM_INOUT");
	}

	FieldRepMap::iterator it1;
	FieldAttrVector::iterator it2;

	if ((fm & PARM_IN) == PARM_IN) {
		for (it1 = inFieldsMap.begin(); it1 != inFieldsMap.end(); it1++) {
			if (it1->second.size() == 0) {
				cout << Fname_1632(it1->first) << " = NULO\n";
				continue;
			} else if (it1->second.size() == 1)
#ifdef _DEBUG
				cout << Fname_1632(it1->first) << "(" << it1->first <<  ") = ["
#else
				cout << Fname_1632(it1->first) << " = ["
#endif
					 << it1->second.begin()->getValue() << "]\n";
			else {
#ifdef _DEBUG
				cout << Fname_1632(it1->first) << "(" << it1->first <<  ") = {\n";
#else
				cout << Fname_1632(it1->first) << " = {\n";
#endif
				for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++) {
					if (Fldtype_1632(it1->first) == FLD_STRING || Fldtype_1632(it1->first) == FLD_CARRAY)
						cout << "\t\"" << it2->getValue() << "\",\n";
					else
						cout << "\t" << it2->getValue() << ",\n";
				}
				cout << "}\n";
			}
		}
	}

	if ((fm & PARM_OUT) == PARM_OUT) {
		for (it1 = outFieldsMap.begin(); it1 != outFieldsMap.end(); it1++) {
			if (it1->second.size() == 0) {
				cout << Fname_1632(it1->first) << " = NULO\n";
				continue;
			} else if (it1->second.size() == 1)
#ifdef _DEBUG
				cout << Fname_1632(it1->first) << "(" << it1->first <<  ") = ["
#else
				cout << Fname_1632(it1->first) << " = ["
#endif
					 << it1->second.begin()->getValue() << "]\n";
			else {
#ifdef _DEBUG
				cout << Fname_1632(it1->first) << "(" << it1->first <<  ") = {\n";
#else
				cout << Fname_1632(it1->first) << " = {\n";
#endif
				for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++) {
					if (Fldtype_1632(it1->first) == FLD_STRING || Fldtype_1632(it1->first) == FLD_CARRAY)
						cout << "\t\"" << it2->getValue() << "\",\n";
					else
						cout << "\t" << it2->getValue() << ",\n";
				}
				cout << "}\n";
			}
		}
	}
}

void GenericTuxedoClient::recycleOutputFields(void)
{
	FieldRepMap::iterator it1, it2;

	for (it1 = outFieldsMap.begin(); it1 != outFieldsMap.end(); it1++) {
		if ((it2 = inFieldsMap.find(it1->first)) != inFieldsMap.end()) {
			it2->second.clear();
			it2->second.assign(it1->second.begin(), it1->second.end());
		}
	}
}

void GenericTuxedoClient::checkEnvVar(string envKey, string envValue) throw (Exception)
{
	if (! envKey.empty() && ! envValue.empty()) {
		string ENV = envKey + "=" + envValue;
		if (tuxputenv(const_cast<char *>(ENV.c_str())) < 0)
			throw Exception("Falha em tuxputenv(\"" + ENV + "\")");
	}

	if (tuxgetenv(const_cast<char *>(envKey.c_str())) == NULL)
		throw Exception("A variavel de ambiente " + envKey + " nao esta presente");

//	cout << envKey << "=[" << tuxgetenv(const_cast<char *>(envKey.c_str())) << "]\n";
}


void GenericTuxedoClient::setInputFields(FieldRepMap inFldMap)
{
	inFieldsMap = inFldMap;
}

void GenericTuxedoClient::setOutputFields(FieldRepMap outFldMap)
{
	outFieldsMap = outFldMap;
}

void GenericTuxedoClient::connect(const string &srvAddr, const string &srvPort)
throw(TPConnectionException)
{
	// codigo que da tpinit no Tuxedo sem nenhuma autenticacao
	// NOTA: sobrecarregar este metodo para os casos possiveis de autenticacao

	if (connected)
		throw TPConnectionException("Ja esta conectado ao TP Monitor");

	if (! srvAddr.empty() && ! srvPort.empty()) {
		checkEnvVar("WSNADDR", "//" + srvAddr + ":" + srvPort);
	}

    if (tpinit((TPINIT *)NULL) < 0)
		throw TPConnectionException(
			string("Falha na conexao ao TP Monitor - ").append(tpstrerror(tperrno)));

	connected = true;
}

void GenericTuxedoClient::disconnect(void)
{
	if (connected) tpterm();
}

void GenericTuxedoClient::run(const string &svcName)
throw(TPFatalException,BufferHandlingException)
{
	if (! connected)
		throw TPFatalException("Nao conectado ao TP Monitor");


	try {
		allocFieldBuffer(PARM_IN);
		allocFieldBuffer(PARM_OUT);
	} catch (BufferHandlingException ex) {
		cout << ex.what();
		throw;
	}

	long succCount = 0, failCount = 0;
	int consecFails = 0;

	while (true) {
		try {
			int result;
			long sizeOfOutBuffer;
			ostringstream fmt;

			populateInputFieldBuffer();
#ifdef _MY_DEBUG
			cout << "svcName=" << svcName.c_str() << "\n";
			cout << "iFieldBuffer: INICIO\n";
			Fprint_1632(iFieldBuffer);
			cout << "iFieldBuffer: FINAL\n";
#endif
			result = tpcall(const_cast<char *>(svcName.c_str()),
							reinterpret_cast<char *>(iFieldBuffer),
							0,
							reinterpret_cast<char **>(&oFieldBuffer),
							&sizeOfOutBuffer,
							TPSIGRSTRT || TPNOTIME);

#ifdef _SHOW_OUTPUT
			cout << "oFieldBuffer: INICIO\n";
			Fprint_1632(oFieldBuffer);
			cout << "oFieldBuffer: FINAL\n";
#endif

			if (result < 0) {
				if (tperrno == TPESVCFAIL || tperrno == TPETIME) {
					if (maxConsecFails > 0 && consecFails >= maxConsecFails) {
						disconnect();
						deleteFieldBuffer(PARM_INOUT);
						fmt << (succCount + failCount + 1) << ": "
							 << "Numero maximo de erros consecutivos chegou ao limite("
							 << maxConsecFails << ")\n";
						break;
					}
					failCount++;
					consecFails++;
					if (tperrno == TPESVCFAIL)
						fmt << "ERRO DE NEGOCIO\n";
					else /* if (tperrno == TPETIME) */
						fmt << "TEMPO ESGOTADO\n";

				} else {
//					cout << tpstrerror(tperrno) << endl;
					fmt.clear();
					fmt << "Erro fatal em run(): " << tpstrerror(tperrno);
					disconnect();
					deleteFieldBuffer(PARM_INOUT);
					throw TPFatalException(fmt.str());
				}
			} else {
				succCount++;
				consecFails = 0;
//				fmt << "OK\n";
			}

			if (!quietMode) {
				if (batchMode) cout << "** " << (succCount + failCount) << ": ";
				cout << fmt.str();
				fmt.str("");
			}

			retrieveOutputFields();
#ifdef _SHOW_OUTPUT
//			showFields(PARM_OUT);
			showFields(PARM_INOUT);
#endif

			if (batchMode && ! endOfBatch) {
				cout << endl;
				recycleOutputFields();
				continue;
			}

			break;
		} catch (...) {
			throw;
		}
	}

	if (batchMode && ! quietMode) {
		cout << "\nEstatisticas de " << svcName << endl;
		cout << "\nCasos de sucesso : " << succCount << endl;
		cout << "Casos de erro    : " << failCount << endl;
	}

	deleteFieldBuffer(PARM_INOUT);
}

void GenericTuxedoClient::addSvcParmIN(string fldName, FLDLEN_1632 length, string value)
{
	addSvcParm(fldName, length, value, PARM_IN);
}

void GenericTuxedoClient::addSvcParmOUT(string fldName, FLDLEN_1632 length, string value)
{
	addSvcParm(fldName, length, value, PARM_OUT);
}

void GenericTuxedoClient::addSvcParmINOUT(string fldName, FLDLEN_1632 length, string value)
{
	addSvcParm(fldName, length, value, PARM_INOUT);
}

string GenericTuxedoClient::getSvcParmValue(string fldName, FLDOCC_1632 fldOcc)
throw (BufferHandlingException)
{
	FLDID_1632 fldId = Fldid_1632(const_cast<char *>(fldName.c_str()));
	FieldRepMap::iterator it1 = outFieldsMap.find(fldId);

	if (it1 == outFieldsMap.end())
		throw BufferHandlingException(
				string("Campo ").append(fldName).append(" não existe"));

	FieldAttrVector &fldVec = outFieldsMap[fldId];
	if (fldOcc >= fldVec.size())
		throw BufferHandlingException(
			string("Núm. de ocorrência inválida para o campo ").append(fldName));

	FieldAttr fldAttr = fldVec[fldOcc];

	return fldAttr.getValue();
}

const FieldRepMap& GenericTuxedoClient::getInFields(void)
{
	return inFieldsMap;
}

const FieldRepMap& GenericTuxedoClient::getOutFields(void)
{
//	FieldRepMap::iterator it;
//	vector<FieldAttr> *vec = new vector<FieldAttr>;

//	for (it = outFieldsMap.begin(); it != outFieldsMap.end(); it++)
//		vec->push_back(it->second);
//	return *vec;
	return outFieldsMap;
}
