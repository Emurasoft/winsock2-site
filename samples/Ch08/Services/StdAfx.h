// stdafx.h : 標準システムインクルードファイル用のインクルードファイル
// 				もしくは、頻繁に使われるがあまり頻繁に変更されない
// 				プロジェクト特有のインクルードファイル
//

#if !defined(AFX_STDAFX_H__951771A9_0217_11D1_85E2_444553540000__INCLUDED_)
#define AFX_STDAFX_H__951771A9_0217_11D1_85E2_444553540000__INCLUDED_

#define _WIN32_WINNT 0x0400

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winsock2.h>
#include <svcguid.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__951771A9_0217_11D1_85E2_444553540000__INCLUDED_)
