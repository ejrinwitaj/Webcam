//////////////////////////////////////////////////////////////////////
// Video Capture using DirectShow
// Author: Shiqi Yu (shiqi.yu@gmail.com)
// Thanks to:
//		HardyAI@OpenCV China
//		flymanbox@OpenCV China (for his contribution to function CameraName, and frame width/height setting)
// Last modification: April 9, 2009
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// ʹ��˵����
//   1. ��CameraDS.h CameraDS.cpp�Լ�Ŀ¼DirectShow���Ƶ������Ŀ��
//   2. �˵� Project->Settings->Settings for:(All configurations)->C/C++->Category(Preprocessor)->Additional include directories
//      ����Ϊ DirectShow/Include
//   3. �˵� Project->Settings->Settings for:(All configurations)->Link->Category(Input)->Additional library directories
//      ����Ϊ DirectShow/Lib
//////////////////////////////////////////////////////////////////////

// CameraDS.cpp: implementation of the CCameraDS class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CameraDS.h"

#pragma comment(lib,"Strmiids.lib") 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDS::CCameraDS()
{
	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;

	m_pFrame = NULL;

	m_pNullFilter = NULL;
	m_pMediaEvent = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pGraph = NULL;

	CoInitialize(NULL);
}

CCameraDS::~CCameraDS()
{
	CloseCamera();
	CoUninitialize();
}

void CCameraDS::CloseCamera()
{
	if(m_bConnected)
	{
		m_pMediaControl->Stop();
	}

	m_pGraph = NULL;
	m_pDeviceFilter = NULL;
	m_pMediaControl = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pSampleGrabber = NULL;
	m_pGrabberInput = NULL;
	m_pGrabberOutput = NULL;
	m_pCameraOutput = NULL;
	m_pMediaEvent = NULL;
	m_pNullFilter = NULL;
	m_pNullInputPin = NULL;

	if (m_pFrame)
	{
		cvReleaseImage(&m_pFrame);
	}

	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;
}

bool CCameraDS::OpenCamera(int nCamID, bool bDisplayProperties, int nWidth, int nHeight)
{
	HRESULT hr = S_OK;

	CoInitialize(NULL);
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGraph);

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **) &m_pMediaControl);
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **) &m_pMediaEvent);

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*) &m_pNullFilter);

	hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);

	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.formattype = FORMAT_VideoInfo; 
	hr = m_pSampleGrabber->SetMediaType(&mt);
	MYFREEMEDIATYPE(mt);

	m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");
 
	// Bind Device Filter.  We know the device because the id was passed in
	BindFilter(nCamID, &m_pDeviceFilter);
	m_pGraph->AddFilter(m_pDeviceFilter, NULL);

	CComPtr<IEnumPins> pEnum;
	m_pDeviceFilter->EnumPins(&pEnum);
 
	hr = pEnum->Reset();
	hr = pEnum->Next(1, &m_pCameraOutput, NULL); 

	pEnum = NULL; 
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pGrabberInput, NULL); 

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	pEnum->Skip(1);
	hr = pEnum->Next(1, &m_pGrabberOutput, NULL); 

	pEnum = NULL;
	m_pNullFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pNullInputPin, NULL);

	//SetCrossBar();

	if (bDisplayProperties) 
	{
		CComPtr<ISpecifyPropertyPages> pPages;

		HRESULT hr = m_pCameraOutput->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
		if (SUCCEEDED(hr))
		{
			PIN_INFO PinInfo;
			m_pCameraOutput->QueryPinInfo(&PinInfo);

			CAUUID caGUID;
			pPages->GetPages(&caGUID);

			OleCreatePropertyFrame(NULL, 0, 0,
						L"Property Sheet", 1,
						(IUnknown **)&(m_pCameraOutput.p),
						caGUID.cElems, caGUID.pElems,
						0, 0, NULL);

			CoTaskMemFree(caGUID.pElems);
			PinInfo.pFilter->Release();
		}
		pPages = NULL;
	}
	else 
	{
		//////////////////////////////////////////////////////////////////////////////
		// ������ lWidth��lHeight���õ�����ͷ�Ŀ��͸� �Ĺ��ܣ�Ĭ��320*240
		// by flymanbox @2009-01-24
		//////////////////////////////////////////////////////////////////////////////
		IAMStreamConfig *iconfig = NULL;
		hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&iconfig);   

		AM_MEDIA_TYPE *pmt;
		if(iconfig->GetFormat(&pmt) !=S_OK)
		{
			//printf("GetFormat Failed ! \n");
			return false;
		}

		// 3�����������ʱ�ĵ�ͼ���С������ nWidth * nHeight����Ͳ����޸��ˡ�
		if ((pmt->lSampleSize != (nWidth * nHeight * 3)) && (pmt->formattype == FORMAT_VideoInfo))
		{
			VIDEOINFOHEADER *phead = (VIDEOINFOHEADER*)(pmt->pbFormat);
			phead->bmiHeader.biWidth = nWidth;
			phead->bmiHeader.biHeight = nHeight;
			if((hr = iconfig->SetFormat(pmt)) != S_OK)
			{
				return false;
			}
		}

		iconfig->Release();
		iconfig=NULL;
		MYFREEMEDIATYPE(*pmt);
	}

	hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);
	hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);

	if (FAILED(hr))
	{
		switch(hr)
		{
			case VFW_S_NOPREVIEWPIN :
				break;
			case E_FAIL :
				break;
			case E_INVALIDARG :
				break;
			case E_POINTER :
				break;
		}
	}

	m_pSampleGrabber->SetBufferSamples(TRUE);
	m_pSampleGrabber->SetOneShot(TRUE);
    
	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if(FAILED(hr))
	{
		return false;
	}

	VIDEOINFOHEADER *videoHeader;
	videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	m_nWidth = videoHeader->bmiHeader.biWidth;
	m_nHeight = videoHeader->bmiHeader.biHeight;
	m_bConnected = true;

	pEnum = NULL;
	return true;
}


bool CCameraDS::BindFilter(int nCamID, IBaseFilter **pFilter)
{
	if (nCamID < 0)
	{
		return false;
	}
 
    // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
		return false;
	}

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) 
	{
		return false;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
	int index = 0;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= nCamID)
    {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				if (index == nCamID)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
    }

	pCreateDevEnum = NULL;
	return true;
}

//������crossbar���PhysConn_Video_Composite
void CCameraDS::SetCrossBar()
{
	int i;
	IAMCrossbar *pXBar1 = NULL;
	ICaptureGraphBuilder2 *pBuilder = NULL;
 
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);

	if (SUCCEEDED(hr))
	{
		hr = pBuilder->SetFiltergraph(m_pGraph);
	}

	hr = pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, m_pDeviceFilter,IID_IAMCrossbar, (void**)&pXBar1);

	if (SUCCEEDED(hr)) 
	{
  		long OutputPinCount, InputPinCount;
		long PinIndexRelated, PhysicalType;
		long inPort = 0, outPort = 0;

		pXBar1->get_PinCounts(&OutputPinCount, &InputPinCount);
		for( i =0;i<InputPinCount;i++)
		{
			pXBar1->get_CrossbarPinInfo(TRUE,i,&PinIndexRelated,&PhysicalType);
			if(PhysConn_Video_Composite==PhysicalType) 
			{
				inPort = i;
				break;
			}
		}
		for( i =0;i<OutputPinCount;i++)
		{
			pXBar1->get_CrossbarPinInfo(FALSE,i,&PinIndexRelated,&PhysicalType);
			if(PhysConn_Video_VideoDecoder==PhysicalType) 
			{
				outPort = i;
				break;
			}
		}
  
		if(S_OK==pXBar1->CanRoute(outPort,inPort))
		{
			pXBar1->Route(outPort,inPort);
		}
		pXBar1->Release();  
	}
	pBuilder->Release();
}

/*
The returned image can not be released.
*/
IplImage* CCameraDS::QueryFrame()
{
	long evCode, size = 0;

	m_pMediaControl->Run();
	m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);
 
	m_pSampleGrabber->GetCurrentBuffer(&size, NULL);

	//if the buffer size changed
	if (size != m_nBufferSize)
	{
		if (m_pFrame)
		{
			cvReleaseImage(&m_pFrame);
		}

		m_nBufferSize = size;
		m_pFrame = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 3);
	}

	m_pSampleGrabber->GetCurrentBuffer(&m_nBufferSize, (long*)m_pFrame->imageData);
	cvFlip(m_pFrame);

	return m_pFrame;
}

int CCameraDS::CameraCount()
{
	int count = 0;
 	CoInitialize(NULL);

   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) 
	{
		return count;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		count++;
    }

	pCreateDevEnum = NULL;
	pEm = NULL;
	return count;
}

int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize)
{
	int count = 0;
 	CoInitialize(NULL);

   // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (hr != NOERROR) return 0;

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		if (count == nCamID)
		{
			IPropertyBag *pBag=0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if(SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //������������,��������Ϣ�ȵ�...
	            if(hr == NOERROR)
		        {
			        //��ȡ�豸����			
					WideCharToMultiByte(CP_ACP,0,var.bstrVal,-1,sName, nBufferSize ,"",NULL);

	                SysFreeString(var.bstrVal);				
		        }
			    pBag->Release();
			}
			pM->Release();

			break;
		}
		count++;
    }

	pCreateDevEnum = NULL;
	pEm = NULL;

	return 1;
}