#ifndef LIBMUEB_GLOBAL_H
#define LIBMUEB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBMUEB_LIBRARY)
#define LIBMUEB_EXPORT Q_DECL_EXPORT
#else
#define LIBMUEB_EXPORT Q_DECL_IMPORT
#endif

class QImage;
class QString;

namespace libmueb::defaults {

extern const int rows;
extern const int roomPerRow;
extern const int windowPerRoom;
extern const int windowPerRow;
extern const int windows;
extern const int windowByteSize;
extern const int verticalPixelUnit;
extern const int horizontalPixelUnit;
extern const quint8 colorDepth;
extern const quint8 factor;
extern const int width;
extern const int height;
extern const QImage frame;
extern const int maxWindowPerDatagram;
extern const int packetHeaderSize;
extern const int packetSize;
extern const int maxPacketNumber;
extern const QString broadcastAddress;
extern const quint16 unicastPort;
extern const quint16 port;

}  // namespace libmueb::defaults

#endif  // LIBMUEB_GLOBAL_H
