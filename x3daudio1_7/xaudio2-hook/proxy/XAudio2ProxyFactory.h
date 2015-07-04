#pragma once

#include <atlbase.h>
#include <atlcom.h>

class ATL_NO_VTABLE XAudio2ProxyFactory :
	public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
	public IClassFactory
{
public:
	DECLARE_NO_REGISTRY()

	BEGIN_COM_MAP(XAudio2ProxyFactory)
		COM_INTERFACE_ENTRY(IClassFactory)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
	HRESULT FinalConstruct() { return S_OK; }
	void FinalRelease() {}

	static HRESULT CreateFactory(IClassFactory* originalFactory, void** proxyFactory);

public:
	STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHODIMP LockServer(BOOL fLock);

private:
	ATL::CComPtr<IClassFactory> m_OriginalFactory;
};
