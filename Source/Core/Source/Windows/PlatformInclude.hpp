#pragma once

#if defined(_WINDOWS_) && !defined(EAO_WINDOWS_INCLUDE)
#pragma message(" ")
#pragma message("You have included windows.h before MinWindows.h")
#pragma message("All useless stuff from the windows headers won't be excluded !!!")
#pragma message(" ")
#endif// _WINDOWS_

#define EAO_WINDOWS_INCLUDE

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS// CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define OEMRESOURCE// OEM Resource values
#define NOATOM// Atom Manager routines
#define NODRAWTEXT// DrawText() and DT_*
#define NOKERNEL// All KERNEL #defines and routines
#define NOMEMMGR// GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE// typedef METAFILEPICT
#define NOMINMAX// Macros min(a,b) and max(a,b)
#define NOOPENFILE// OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL// SB_* and scrolling routines
#define NOSERVICE// All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND// Sound driver routines
#define NOCOMM// COMM driver routines
#define NOKANJI// Kanji support stuff.
#define NOHELP// Help engine interface.
#define NOPROFILER// Profiler interface.
#define NODEFERWINDOWPOS// DeferWindowPos routines
#define NOMCX// Modem Configuration Extensions
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC

struct IUnknown;

#if __has_include(<boost/asio.hpp>)
// Finally now we can include windows.h
// Set the proper SDK version before including boost/Asio
#include <SDKDDKVer.h>
// Note boost/ASIO includes Windows.h.
#include <boost/asio.hpp>
#else
#include <Windows.h>
#endif

/* Wincrypt must be included before anything that could include OpenSSL. */
#include <wincrypt.h>
/* Undefine wincrypt conflicting symbols for BoringSSL. */
#undef X509_NAME
#undef X509_EXTENSIONS
#undef PKCS7_ISSUER_AND_SERIAL
#undef PKCS7_SIGNER_INFO
#undef OCSP_REQUEST
#undef OCSP_RESPONSE

#undef far
#undef near