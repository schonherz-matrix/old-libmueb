#ifndef LIBMUEBCONFIG_H
#define LIBMUEBCONFIG_H

#include <QImage>
#include <QString>
#include <cmath>

namespace libmueb::defaults {

inline constexpr quint32 rows = 13;
inline constexpr quint32 roomPerRow = 8;
inline constexpr quint32 windowPerRoom = 2;
inline constexpr quint32 windowPerRow = roomPerRow * windowPerRoom;
inline constexpr quint32 windows = rows * windowPerRow;
inline constexpr quint32 verticalPixelUnit = 2;
inline constexpr quint32 horizontalPixelUnit = 2;
inline constexpr quint8 colorDepth = 3;
inline constexpr quint32 windowByteSize =
    (colorDepth == 3 || colorDepth == 4)
        ? horizontalPixelUnit * verticalPixelUnit * 3 / 2
        : horizontalPixelUnit * verticalPixelUnit * 3;
inline constexpr quint32 width = windowPerRow * horizontalPixelUnit;
inline constexpr quint32 height = rows * verticalPixelUnit;
inline constexpr quint32 maxWindowPerDatagram = 208;
inline constexpr quint32 packetHeaderSize = 2;
inline constexpr quint32 packetSize =
    maxWindowPerDatagram * windowByteSize + packetHeaderSize;
inline const quint32 maxPacketNumber =
    std::ceil(windows / maxWindowPerDatagram);
inline const QImage frame{width, height, QImage::Format_RGB888};
inline constexpr quint32 protocolType =
    (maxWindowPerDatagram == windows) ? 2 : 1;
inline constexpr quint16 unicastPort = 3000;
inline constexpr quint16 port = 10000;
inline const QString broadcastAddress{"10.6.255.255"};
inline constexpr quint8 factor = 8 - colorDepth;

}  // namespace libmueb::defaults

#endif  // LIBMUEBCONFIG_H
