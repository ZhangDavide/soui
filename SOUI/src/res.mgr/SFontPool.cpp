//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "res.mgr/sfontpool.h"
#include "helper/SplitString.h"
#include "helper/spropbag.h"

namespace SOUI
{

template<> SFontPool* SSingleton<SFontPool>::ms_Singleton    = 0;

static LPCTSTR KDefFaceName = _T("宋体");

SFontPool::SFontPool(IRenderFactory *pRendFactory)
    :m_RenderFactory(pRendFactory)
{
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);
    m_lfDefault.lfHeight  = -12;
    m_lfDefault.lfQuality = CLEARTYPE_QUALITY;
    _tcscpy(m_lfDefault.lfFaceName,KDefFaceName);

    m_pFunOnKeyRemoved=OnKeyRemoved;
    SetKeyObject(FontKey(0),_CreateFont(m_lfDefault));
}

IFontPtr SFontPool::GetFont(FONTSTYLE style, const SStringT & pszFaceName,const IPropBag * pPropEx)
{
    SStringT strFaceName(pszFaceName);
    if(strFaceName == m_lfDefault.lfFaceName) strFaceName = _T("");
    
    IFontPtr hftRet=0;
	SStringT strPropEx;
	if(pPropEx) strPropEx = pPropEx->ToXml();

    FontKey key(style.dwStyle,strFaceName,strPropEx);
    if(HasKey(key))
    {
        hftRet=GetKeyObject(key);
    }
    else
    {
        if(strFaceName.IsEmpty()) strFaceName = m_lfDefault.lfFaceName;
        hftRet = _CreateFont(style,strFaceName,pPropEx);
        AddKeyObject(key,hftRet);
    }
    return hftRet;
}

static const TCHAR  KFontPropSeprator=  _T(',');   //字体属性之间的分隔符，不再支持其它符号。
static const TCHAR  KPropSeprator    =  _T(':');   //一个属性name:value对之间的分隔符
static const TCHAR  KAttrFalse[]     =   _T("0");
static const TCHAR  KFontFace[]      =   _T("face");
static const TCHAR  KFontBold[]      =   _T("bold");
static const TCHAR  KFontUnderline[] =   _T("underline");
static const TCHAR  KFontItalic[]    =   _T("italic");
static const TCHAR  KFontStrike[]    =   _T("strike");
static const TCHAR  KFontAdding[]    =   _T("adding");
static const TCHAR  KFontSize[]      =   _T("size");
static const TCHAR  KFontCharset[]   =   _T("charset");


#define LEN_FACE    (ARRAYSIZE(KFontFace)-1)
#define LEN_BOLD    (ARRAYSIZE(KFontBold)-1)
#define LEN_UNDERLINE    (ARRAYSIZE(KFontUnderline)-1)
#define LEN_ITALIC  (ARRAYSIZE(KFontItalic)-1)
#define LEN_STRIKE  (ARRAYSIZE(KFontStrike)-1)
#define LEN_ADDING  (ARRAYSIZE(KFontAdding)-1)
#define LEN_SIZE    (ARRAYSIZE(KFontSize)-1)
#define LEN_CHARSET (ARRAYSIZE(KFontCharset)-1)

IFontPtr SFontPool::GetFont( const SStringW & strFont )
{
    FONTSTYLE fntStyle(0);
    fntStyle.byCharset = DEFAULT_CHARSET;
    
    SStringT strFace;                                         
    SStringT attr=S_CW2T(strFont);                           
    attr.MakeLower();                                        
    SStringTList fontProp;
    SplitString(attr,KFontPropSeprator,fontProp);
	SPropBag propBag;
    for(int i=fontProp.GetCount()-1;i>=0;i--)
    {
        SStringTList strPair;
        if(2!=SplitString(fontProp[i],KPropSeprator,strPair))
        {
            fontProp.RemoveAt(i);
            continue;
        }
        if(strPair[0] == KFontFace)
        {
            strFace = strPair[1];
        }else if(strPair[0] == KFontAdding)
        {
            fntStyle.cSize=(short)_ttoi(strPair[1]); 
        }else if(strPair[0] == KFontSize)
        {
            fntStyle.cSize=(short)_ttoi(strPair[1]); 
            fntStyle.fAbsSize = 1;
        }else if(strPair[0] == KFontItalic)
        {
            fntStyle.fItalic = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontBold)
        {
            fntStyle.fBold = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontStrike)
        {
            fntStyle.fStrike = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontUnderline)
        {
            fntStyle.fUnderline = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontCharset)
        {
            fntStyle.byCharset = (BYTE)_ttoi(strPair[1]);
        }else
		{
			propBag.SetKeyValue(strPair[0],strPair[1]);
		}
    }
    return GetFont(fntStyle, strFace, &propBag);
}

#define HASFONT 2
int CALLBACK DefFontsEnumProc(  CONST LOGFONT *lplf,     // logical-font data
                              CONST TEXTMETRIC *lptm,  // physical-font data
                              DWORD dwType,            // font type
                              LPARAM lpData            // application-defined data
                              )
{
    return HASFONT;
}

void SFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize,BYTE byCharset/*=DEFAULT_CHARSET*/)
{
    //初始化前才可以调用该接口
    if(GetCount()>1) return;
    RemoveKeyObject(FontKey(0));

    HDC hdc = GetDC(NULL);
    int hasFont = EnumFonts(hdc,lpszFaceName,DefFontsEnumProc,0);
    ReleaseDC(NULL,hdc);
    if(hasFont == HASFONT)
        _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),lpszFaceName);
    else
        _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),_T("宋体"));
    
    m_lfDefault.lfHeight = -abs(lSize);
    if(byCharset!=DEFAULT_CHARSET) 
        m_lfDefault.lfCharSet = byCharset;
    
    SetKeyObject(FontKey(0),_CreateFont(m_lfDefault));
}


IFontPtr SFontPool::_CreateFont(const LOGFONT &lf,const IPropBag * pPropBag )
{
    
    SASSERT(m_RenderFactory);
    
    
    IFontPtr pFont=NULL;
    m_RenderFactory->CreateFont(&pFont,lf,pPropBag);

    return pFont;
}

IFontPtr SFontPool::_CreateFont(FONTSTYLE style,const SStringT & strFaceName,const IPropBag * pPropBag)
{
    LOGFONT lfNew;
        
    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    lfNew.lfWeight      = (style.fBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != style.fUnderline);
    lfNew.lfItalic      = (FALSE != style.fItalic);
    lfNew.lfStrikeOut   = (FALSE != style.fStrike);
    if(style.fAbsSize)
        lfNew.lfHeight = -abs((short)style.cSize);
    else
        lfNew.lfHeight -= (short)style.cSize;  //cSize为正代表字体变大，否则变小
        
    lfNew.lfQuality = CLEARTYPE_QUALITY;
    
    if(style.byCharset!=DEFAULT_CHARSET) lfNew.lfCharSet = style.byCharset;
    
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);
    
    return _CreateFont(lfNew,pPropBag);
}


}//namespace SOUI