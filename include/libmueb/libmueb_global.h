#ifndef LIBMUEB_GLOBAL_H
#define LIBMUEB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef LIBMUEB_AS_DLL
#if defined(LIBMUEB_LIBRARY)
#define LIBMUEB_EXPORT Q_DECL_EXPORT
#else
#define LIBMUEB_EXPORT Q_DECL_IMPORT
#endif
#else
#define LIBMUEB_EXPORT
#endif

class QImage;
class QString;

namespace libmueb::defaults {

LIBMUEB_EXPORT extern const int rows;
LIBMUEB_EXPORT extern const int roomPerRow;
LIBMUEB_EXPORT extern const int windowPerRoom;
LIBMUEB_EXPORT extern const int windowPerRow;
LIBMUEB_EXPORT extern const int windows;
LIBMUEB_EXPORT extern const int windowByteSize;
LIBMUEB_EXPORT extern const int verticalPixelUnit;
LIBMUEB_EXPORT extern const int horizontalPixelUnit;
LIBMUEB_EXPORT extern const quint8 colorDepth;
LIBMUEB_EXPORT extern const quint8 factor;
LIBMUEB_EXPORT extern const int width;
LIBMUEB_EXPORT extern const int height;
LIBMUEB_EXPORT extern const QImage frame;
LIBMUEB_EXPORT extern const int maxWindowPerDatagram;
LIBMUEB_EXPORT extern const int packetHeaderSize;
LIBMUEB_EXPORT extern const int packetSize;
LIBMUEB_EXPORT extern const int maxPacketNumber;
LIBMUEB_EXPORT extern const QString broadcastAddress;
LIBMUEB_EXPORT extern const quint16 unicastPort;
LIBMUEB_EXPORT extern const quint16 port;
LIBMUEB_EXPORT extern const int protocolType;

}  // namespace libmueb::defaults

#endif  // LIBMUEB_GLOBAL_H
