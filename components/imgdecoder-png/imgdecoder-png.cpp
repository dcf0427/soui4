﻿
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "imgdecoder-png.h"
#include "decoder-apng.h"
#include <png.h>

namespace SOUI
{
    //////////////////////////////////////////////////////////////////////////
    //  SImgFrame_PNG
    SImgFrame_PNG::SImgFrame_PNG()
        :m_pdata(NULL)
        ,m_nWid(0)
        ,m_nHei(0)
        ,m_nFrameDelay(0)
    {

    }


    void SImgFrame_PNG::Attach( const BYTE * pdata,int nWid,int nHei,int nDelay )
    {
        m_pdata=(pdata);
        m_nWid=(nWid);
        m_nHei=(nHei);
        m_nFrameDelay=(nDelay);
    }

    BOOL SImgFrame_PNG::GetSize( UINT *pWid,UINT *pHei )
    {
        if(!m_pdata) return FALSE;
        *pWid = m_nWid;
        *pHei = m_nHei;
        return TRUE;
    }

    const VOID * SImgFrame_PNG::GetPixels(CTHIS) SCONST{
        return m_pdata;
    }
    //////////////////////////////////////////////////////////////////////////
    // SImgX_PNG
    int SImgX_PNG::LoadFromMemory( void *pBuf,size_t bufLen )
    {
        APNGDATA * pdata =LoadAPNG_from_memory((char*)pBuf,bufLen);
        return _DoDecode(pdata);
    }

    int SImgX_PNG::LoadFromFileW( LPCWSTR pszFileName )
    {
        APNGDATA * pdata =LoadAPNG_from_file(pszFileName);
        return _DoDecode(pdata);
    }

    int SImgX_PNG::LoadFromFileA( LPCSTR pszFileName )
    {
        APNGDATA * pdata =LoadAPNG_from_fileA(pszFileName);
        return _DoDecode(pdata);
    }

    SImgX_PNG::SImgX_PNG( BOOL bPremultiplied )
        :m_bPremultiplied(bPremultiplied)
        ,m_pImgArray(NULL)
        ,m_pngData(NULL)
    {

    }

    SImgX_PNG::~SImgX_PNG( void )
    {
        if(m_pImgArray) delete []m_pImgArray;
        m_pImgArray = NULL;
        if(m_pngData) APNG_Destroy(m_pngData);
    }

    int SImgX_PNG::_DoDecode(APNGDATA *pData)
    {
        if(!pData) return 0;
        m_pngData = pData;

        int nWid = m_pngData->nWid;
        int nHei = m_pngData->nHei;

        //swap rgba to bgra and do premultiply
        BYTE *p=m_pngData->pdata;
        int pixel_count = nWid * nHei * m_pngData->nFrames;
        for (int i=0; i < pixel_count; ++i) {
            BYTE a = p[3];
            BYTE t = p[0];
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

        p=m_pngData->pdata;
        m_pImgArray = new SImgFrame_PNG[m_pngData->nFrames];
        for(int i=0;i<m_pngData->nFrames;i++)
        {
            m_pImgArray[i].Attach(p,nWid,nHei,m_pngData->pDelay?m_pngData->pDelay[i]:0);
            p += nWid*nHei*4;
        }
        return m_pngData->nFrames;
    }

    UINT SImgX_PNG::GetFrameCount()
    {
        return m_pngData?m_pngData->nFrames:0;
    }

	IImgFrame * SImgX_PNG::GetFrame(UINT iFrame)
	{
		if(iFrame >= GetFrameCount()) return NULL;
		return m_pImgArray+iFrame;
	}

    //////////////////////////////////////////////////////////////////////////
    //  SImgDecoderFactory_PNG

    SImgDecoderFactory_PNG::SImgDecoderFactory_PNG()
    {

    }

    SImgDecoderFactory_PNG::~SImgDecoderFactory_PNG()
    {

    }

    BOOL SImgDecoderFactory_PNG::CreateImgX( IImgX **ppImgDecoder )
    {
        *ppImgDecoder = new SImgX_PNG(TRUE);
        return TRUE;
    }
    
  
	HRESULT SImgDecoderFactory_PNG::SaveImage(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, const void* pFormat) SCONST
    {
        if(!pBits) return E_INVALIDARG;
                
        FILE *fp;  
        png_structp png_ptr;  
        png_infop info_ptr;  

        /* 打开需要写入的文件 */  
        #ifdef _WIN32
        fp = _wfopen(pszFileName, L"wb");  
        #else
        char szFileName[1000];
        WideCharToMultiByte(CP_UTF8,0,pszFileName,-1,szFileName,1000,NULL,NULL);
        fp = fopen(szFileName,"wb");
        #endif
        if (fp == NULL)  
            return (E_FAIL);  

        /* 创建并初始化 png_struct 及其所需的错误处理函数，如果你想使用默 
        * 认的 stderr 和 longjump() 方法，你可以将最后三个参数设为 NULL， 
        * 在使用动态链接库的情况下，我们也会检测函数库版本是否与在编译时 
        * 使用的版本是否兼容。（必要） 
        */  
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  

        if (png_ptr == NULL)  
        {  
            fclose(fp);  
            return (E_FAIL);  
        }  

        /* 分配内存并初始化图像信息数据。（必要）*/  
        info_ptr = png_create_info_struct(png_ptr);  
        if (info_ptr == NULL)  
        {  
            fclose(fp);  
            png_destroy_write_struct(&png_ptr,  NULL);  
            return (E_FAIL);  
        }  

        /* 设置错误处理。如果你在调用 png_create_write_struct() 时没 
        * 有设置错误处理函数，那么这段代码是必须写的。*/  
        if (setjmp(png_jmpbuf(png_ptr)))  
        {  
            /* 如果程序跑到这里了，那么写入文件时出现了问题 */  
            fclose(fp);  
            png_destroy_write_struct(&png_ptr, &info_ptr);  
            return (E_FAIL);  
        }  

        /* 设置输出控制，如果你使用的是 C 的标准 I/O 流 */  
        png_init_io(png_ptr, fp);  

        /* 这是一种复杂的做法 */  

        /* （必需）在这里设置图像的信息，宽度、高度的上限是 2^31。 
        * bit_depth 取值必需是 1、2、4、8 或者 16, 但是可用的值也依赖于 color_type。 
        * color_type 可选值有： PNG_COLOR_TYPE_GRAY、PNG_COLOR_TYPE_GRAY_ALPHA、 
        * PNG_COLOR_TYPE_PALETTE、PNG_COLOR_TYPE_RGB、PNG_COLOR_TYPE_RGB_ALPHA。 
        * interlace 可以是 PNG_INTERLACE_NONE 或 PNG_INTERLACE_ADAM7, 
        * 而 compression_type 和 filter_type 目前必需是 PNG_COMPRESSION_TYPE_BASE 
        * 和 and PNG_FILTER_TYPE_BASE。 
        */  

        png_uint_32 width = nWid;
        png_uint_32 height = nHei;
        png_uint_32 bytes_per_pixel = 4;

        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,  
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);  

        /* 写入文件头部信息（必需） */  
        png_write_info(png_ptr, info_ptr);  

        /* 将原数据格式从预乘的rgba格式调整为不预乘的bgra格式 */  
        png_bytep image = (png_bytep) new png_byte[width*height*bytes_per_pixel];  
        png_bytep dst=image;
        png_bytep src = (png_bytep)pBits;
        for(png_uint_32 y=0;y<height;y++) for(png_uint_32 x=0;x<width;x++)
        {
            png_byte a = src[3];
            dst[2] = (png_byte)((int)src[0]*a/255);
            dst[1] = (png_byte)((int)src[1]*a/255);
            dst[0] = (png_byte)((int)src[2]*a/255);
            dst[3]=a;
            dst += 4;
            src += 4;
        }
        
        if (height > PNG_UINT_32_MAX/(sizeof (png_bytep)))  
            png_error (png_ptr, "Image is too tall to process in memory");  

        png_bytep *row_pointers = new png_bytep[height];  


        /* 将这些像素行指针指向你的 "image" 字节数组中对应的位置，即：指向每行像素的起始处 */  
        for (png_uint_32 k = 0; k < height; k++)  
            row_pointers[k] = image + k*width*bytes_per_pixel;  

        /* 一次调用就将整个图像写进文件 */  
        png_write_image(png_ptr, row_pointers);  
        /* 必需调用这个函数完成写入文件其余部分 */  
        png_write_end(png_ptr, info_ptr);  
        /* 写完后清理并释放已分配的内存 */  
        png_destroy_write_struct(&png_ptr, &info_ptr);  

        delete []row_pointers;
        /* 关闭文件 */  
        fclose(fp);  

        return S_OK;
    }

	HRESULT SImgDecoderFactory_PNG::SaveImage2(THIS_ BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) const
	{
		if(imgFmt!=Img_PNG)
			return E_INVALIDARG;
		return SaveImage(pBits,nWid,nHei,pszFileName,NULL);
	}

    LPCWSTR SImgDecoderFactory_PNG::GetDescription() const
    {
        return DESC_IMGDECODER;
    }

    //////////////////////////////////////////////////////////////////////////
    BOOL IMGDECODOR_PNG::SCreateInstance( IObjRef **ppImgDecoderFactory )
    {
        *ppImgDecoderFactory = new SImgDecoderFactory_PNG();
        return TRUE;
    }


}//end of namespace SOUI

EXTERN_C BOOL Decoder_Png_SCreateInstance(IObjRef **ppImgDecoderFactory)
{
	return SOUI::IMGDECODOR_PNG::SCreateInstance(ppImgDecoderFactory);
}
