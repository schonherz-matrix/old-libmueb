#include "libmueb_global.h"

#include <QImage>
#include <QString>
#include <QtMath>

namespace libmueb::defaults {

extern const int rows = 13;
extern const int roomPerRow = 8;
extern const int windowPerRoom = 2;
extern const int windowPerRow = roomPerRow * windowPerRoom;
extern const int windows = rows * windowPerRow;
extern const int windowByteSize =
    (colorDepth == 3 || colorDepth == 4)
        ? horizontalPixelUnit * verticalPixelUnit * 3 / 2
        : horizontalPixelUnit * verticalPixelUnit * 3;
extern const int verticalPixelUnit = 2;
extern const int horizontalPixelUnit = 2;
extern const quint8 colorDepth = 3;
extern const quint8 factor = 8 - colorDepth;
extern const int width = windowPerRow * horizontalPixelUnit;
extern const int height = rows * verticalPixelUnit;
extern const QImage frame{width, height, QImage::Format_RGB888};
extern const int maxWindowPerDatagram = 208;
extern const int packetHeaderSize = 2;
extern const int packetSize =
    maxWindowPerDatagram * windowByteSize + packetHeaderSize;
extern const int maxPacketNumber = qCeil(windows / maxWindowPerDatagram);
extern const QString broadcastAddress{"10.6.255.255"};
extern const quint16 unicastPort = 3000;
extern const quint16 port = 10000;
extern const int protocolType = (maxWindowPerDatagram == windows) ? 2 : 1;

}  // namespace libmueb::defaults
