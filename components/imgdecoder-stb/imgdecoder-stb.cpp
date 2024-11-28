﻿
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "imgdecoder-stb.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace SOUI
{
    //////////////////////////////////////////////////////////////////////////
    //  SImgFrame_STB
    SImgFrame_STB::SImgFrame_STB(BYTE *data,int w,int h):m_data(data),m_nWid(w),m_nHei(h)
    {

    }

    SImgFrame_STB::~SImgFrame_STB()
    {
        if(m_data) free(m_data);
    }

    BOOL SImgFrame_STB::GetSize( UINT *pWid,UINT *pHei )
    {
        *pWid = m_nWid;
        *pHei = m_nHei;
        return TRUE;
    }

    const VOID * SImgFrame_STB::GetPixels(CTHIS) SCONST{
        return m_data;
    }
    //////////////////////////////////////////////////////////////////////////
    // SImgX_STB
    int SImgX_STB::LoadFromMemory( void *pBuf,size_t bufLen )
    {
        if(m_pImg) delete m_pImg;
        m_pImg = NULL;

        if(!pBuf) return 0;

        int w=0,h=0;
        unsigned char *data = stbi_load_from_memory((stbi_uc const *)pBuf,bufLen,&w,&h,NULL,4);
        if(!data)
        {
            return 0;
        }
        _DoPromultiply(data,w,h);
        m_pImg = new SImgFrame_STB(data,w,h);
        return 1;
    }

    int SImgX_STB::LoadFromFileW( LPCWSTR pszFileName )
    {
        if(m_pImg) delete m_pImg;
        m_pImg = NULL;
#ifdef _WIN32
        FILE *f=_wfopen(pszFileName,L"rb");
#else
        char szFileName[1000]={0};
        WideCharToMultiByte(CP_UTF8,0,pszFileName,-1,szFileName,1000,NULL,NULL);
        FILE *f = fopen(szFileName,"rb");
#endif
        if(!f) return 0;
        int w=0,h=0;
        unsigned char *data = stbi_load_from_file(f,&w,&h,NULL,4);
        fclose(f);
        if(!data)
        {
            return 0;
        }
        _DoPromultiply(data,w,h);
        m_pImg = new SImgFrame_STB(data,w,h);
        return 1;
    }

    #ifndef MAX_PATH
    #define MAX_PATH 1000
    #endif
    int SImgX_STB::LoadFromFileA( LPCSTR pszFileName )
    {
        wchar_t wszFileName[MAX_PATH+1];
        int cp = CP_ACP;
    #ifndef _WIN32
        cp = CP_UTF8;
    #endif
        MultiByteToWideChar(cp,0,pszFileName,-1,wszFileName,MAX_PATH);
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER) return 0;
        return LoadFromFileW(wszFileName);
    }

    SImgX_STB::SImgX_STB(BOOL bPremultiple)
        :m_pImg(NULL)
        ,m_bPremultiple(bPremultiple)
    {

    }

    SImgX_STB::~SImgX_STB( void )
    {
        if(m_pImg) delete m_pImg;
    }

    void SImgX_STB::_DoPromultiply( BYTE *pdata,int nWid,int nHei )
    {
        //swap rgba to bgra and do premultiply
        BYTE *p=pdata;
        int pixel_count = nWid * nHei;
        for (int i=0; i < pixel_count; ++i) {
            stbi_uc a = p[3];
            stbi_uc t = p[0];
            if (a) 
            {
                p[0] = (p[2] *a)/255;
                p[1] = (p[1] * a)/255;
                p[2] =  (t   * a)/255;
            }else
            {
                memset(p,0,4);
            }
            p += 4;
        }    
    }

	IImgFrame * SImgX_STB::GetFrame(UINT iFrame)
	{
		if(iFrame >= GetFrameCount()) return NULL;
		return m_pImg;
	}

	UINT SImgX_STB::GetFrameCount()
	{
		return m_pImg?1:0;
	}

    //////////////////////////////////////////////////////////////////////////
    //  SImgDecoderFactory_STB

    SImgDecoderFactory_STB::SImgDecoderFactory_STB( )
    {
    }

    SImgDecoderFactory_STB::~SImgDecoderFactory_STB()
    {

    }

    BOOL SImgDecoderFactory_STB::CreateImgX( IImgX **ppImgDecoder )
    {
        *ppImgDecoder = new SImgX_STB(TRUE);
        return TRUE;
    }
    
    LPCWSTR SImgDecoderFactory_STB::GetDescription() const
    {
        return DESC_IMGDECODER;
    }

	HRESULT SImgDecoderFactory_STB::SaveImage(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, const void * pFormat) SCONST
	{
		return E_NOTIMPL;
	}

	HRESULT SImgDecoderFactory_STB::SaveImage2(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
		return E_NOTIMPL;
	}

    //////////////////////////////////////////////////////////////////////////
    BOOL IMGDECODOR_STB::SCreateInstance( IObjRef **pImgDecoderFactory )
    {
        *pImgDecoderFactory = new SImgDecoderFactory_STB();
        return TRUE;
    }

}//end of namespace SOUI

EXTERN_C BOOL Decoder_Stb_SCreateInstance(IObjRef **pImgDecoderFactory)
{
	return SOUI::IMGDECODOR_STB::SCreateInstance(pImgDecoderFactory);
}
