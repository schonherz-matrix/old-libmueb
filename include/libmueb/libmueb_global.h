#pragma once

#include <QtCore/qglobal.h>
#include <libmuebconfig.h>

#ifdef LIBMUEB_AS_DLL
#if defined(LIBMUEB_LIBRARY)
#define LIBMUEB_EXPORT Q_DECL_EXPORT
#else
#define LIBMUEB_EXPORT Q_DECL_IMPORT
#endif
#else
#define LIBMUEB_EXPORT
#endif
