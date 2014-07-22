package br.com.teclatab.tux4j;

import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.File;

public class GenericClient
{
	private static final String DLL_NAME = "/tux4j.dll";
	private static final String DEFAULT_SVRPORT = "65432";
	private static final int PARM_IN		= 1;
	private static final int PARM_OUT		= 2;
	private static final int PARM_INOUT		= 3;

	private boolean connected;
	private String tuxedoClientDir;
	private String fmlTableDir;
	private String fmlTables;

	public GenericClient() {connected = false;}

	public void setTuxedoClientDir(String value)
	{
		if (value != null) tuxedoClientDir = value;
	}


	public void setFMLTableDir(String value)
	{
		if (value != null) fmlTableDir = value;
	}

	public void setFMLTables(String value)
	{
		if (value != null) fmlTables = value;
	}

	public String getTuxedoClientDir()	{return tuxedoClientDir;}
	public String getFMLTableDir()		{return fmlTableDir;}
	public String getFMLTables()		{return fmlTables;}

	public void connect(String svrAddr) throws TPConnectionException
	{
		connect(svrAddr, DEFAULT_SVRPORT);
	}

	public void connect(String svrAddr, String svrPort) throws TPConnectionException
	{
		if (svrAddr == null || svrAddr.equals(""))
			throw new TPConnectionException("O endereço do servidor deve ser informado");

		if (svrPort == null || svrPort.equals(""))
			throw new TPConnectionException("A porta TCP do servidor deve ser informada");

		tuxConnect(svrAddr, svrPort);
		connected = true;
	}

	public void disconnect()
	{
		tuxDisconnect();
		connected = false;
	}

	public void run(String svcName) throws TPFatalException, BufferHandlingException
	{
		if (svcName == null || svcName.equals(""))
			throw new TPFatalException("O nome do serviço deve ser informado");
		tuxRun(svcName);
	}

	public void addParmIN(String fldName, long fldLength, String fldValue)
		throws BufferHandlingException
	{
		if (fldName == null || fldName.equals(""))
			throw new BufferHandlingException("O nome do campo deve ser informado");
		if (fldValue == null) fldValue = "";
		bufAddField(fldName, fldLength, fldValue, PARM_IN);
	}

	public void addParmINOUT(String fldName, long fldLength, String fldValue)
		throws BufferHandlingException
	{
		if (fldName == null || fldName.equals(""))
			throw new BufferHandlingException("O nome do campo deve ser informado");
		if (fldValue == null) fldValue = "";
		bufAddField(fldName, fldLength, fldValue, PARM_INOUT);
	}

	public void addParmOUT(String fldName, long fldLength)
		throws BufferHandlingException
	{
		if (fldName == null || fldName.equals(""))
			throw new BufferHandlingException("O nome do campo deve ser informado");
		bufAddField(fldName, fldLength, "", PARM_OUT);
	}

	public String getParmValue(String fldName) throws BufferHandlingException
	{
		if (fldName == null || fldName.equals(""))
			throw new BufferHandlingException("O nome do campo deve ser informado");
		return bufGetFieldValue(fldName, 0L);
	}

	public String getParmValue(String fldName, long fldOcc) throws BufferHandlingException
	{
		if (fldName == null || fldName.equals(""))
			throw new BufferHandlingException("O nome do campo deve ser informado");
		return bufGetFieldValue(fldName, fldOcc);
	}

//	public long getParmOccurences(String fldName)
//	{
//		return bufGetFieldOcc(fldName);
//	}

	public void finalize() throws Throwable
	{
		super.finalize();
		disconnect();
	}

	private native void tuxConnect(String svrAddr, String svrPort)
		throws TPConnectionException;

	private native void tuxDisconnect();

	private native void tuxRun(String svcName)
		throws TPFatalException, BufferHandlingException;

	private native void bufAddField(String fldName, long fldLength,
									String fldValue, int fldMode)
		throws BufferHandlingException;

	private native String bufGetFieldValue(String fldName, long fldOcc)
		throws BufferHandlingException;

//	private native long bufGetFieldOcc(String fldName)
//		throws BufferHandlingException;

	static {
		try {
			InputStream inputStream =
				GenericClient.class.getResource(DLL_NAME).openStream();
			File tmpDll = File.createTempFile("tux4j", ".dll");
			FileOutputStream out = new FileOutputStream(tmpDll);
			byte[] buff = new byte[8192];
			int bytesRead = inputStream.read(buff);
			while (bytesRead != -1) {
				out.write(buff, 0, bytesRead);
				bytesRead = inputStream.read(buff);
			}
			out.close();
			tmpDll.deleteOnExit();
			System.load(tmpDll.getPath());
		} catch (Exception e) {
			System.out.println("Erro na carga da DLL");
			System.out.println(e);
			e.printStackTrace();
		}
	}
}

/*

class FmlBuffer
{
    public abstract void Finit() {}
    public abstract void Ferr() {}
    public abstract void CFchg() {}
    public abstract void CFfind() {}
}

class Fml16Buffer extends FmlBuffer
{
    public void Finit() {}
    public void Ferr() {}
    public void CFchg() {}
    public void CFfind() {}
}

class Fml32Buffer extends FmlBuffer
{
    public void Finit() {}
    public void Ferr() {}
    public void CFchg() {}
    public void CFfind() {}
}

class TuxService
{
    private String svcname;
    private FmlBuffer svcbuf;
    private String errmsg;
    private int errno;
    private TuxConnection tuxconn;

    public boolean call() {}

    // USA: atmi.h

}

*/
