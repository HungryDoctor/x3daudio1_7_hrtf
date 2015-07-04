#include "stdafx.h"
#include "XAudio2ProxyFactory.h"
#include "XAudio2Proxy.h"
#include <stdexcept>

STDMETHODIMP XAudio2ProxyFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;

	ATL::CComPtr<IUnknown> originalObject;
	HRESULT hr = m_OriginalFactory->CreateInstance(pUnkOuter, riid, (void**)&originalObject);
	if (FAILED(hr))
		return hr;

	return XAudio2Proxy::CreateInstance(originalObject, riid, ppvObject);
}

STDMETHODIMP XAudio2ProxyFactory::LockServer(BOOL fLock)
{
	return m_OriginalFactory->LockServer(fLock);
}

HRESULT XAudio2ProxyFactory::CreateFactory(IClassFactory* originalFactory, void** proxyFactory)
{
	try
	{
		ATL::CComObjectNoLock<XAudio2ProxyFactory>* self = new ATL::CComObjectNoLock<XAudio2ProxyFactory>;

		self->SetVoid(NULL);

		self->InternalFinalConstructAddRef();
		HRESULT hr = self->_AtlInitialConstruct();
		if (SUCCEEDED(hr))
			hr = self->FinalConstruct();
		if (SUCCEEDED(hr))
			hr = self->_AtlFinalConstruct();
		self->InternalFinalConstructRelease();

		self->m_OriginalFactory = originalFactory;

		if (SUCCEEDED(hr))
			hr = self->QueryInterface(IID_IClassFactory, proxyFactory);

		if (hr != S_OK)
			delete self;

		return hr;
	}
	catch (std::bad_alloc&)
	{
		return E_OUTOFMEMORY;
	}
}