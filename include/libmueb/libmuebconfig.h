#ifndef LIBMUEBCONFIG_H
#define LIBMUEBCONFIG_H

#include <QImage>
#include <QString>
#include <cmath>

namespace libmueb {

enum class Mode { WINDOW_WISE, ROW_WISE };

namespace defaults {

// Hardware specific constants
inline constexpr quint32 rows = 13;
inline constexpr quint32 roomPerRow = 8;
inline constexpr quint32 windowPerRoom = 2;
inline constexpr quint32 verticalPixelUnit = 2;
inline constexpr quint32 horizontalPixelUnit = 2;
inline constexpr quint8 colorDepth = 3;

// Calculated, software specific constants
inline constexpr quint32 pixelPerWindow =
    verticalPixelUnit * horizontalPixelUnit;
inline constexpr quint32 windowPerRow = roomPerRow * windowPerRoom;
inline constexpr quint32 windows = rows * windowPerRow;
inline constexpr quint32 pixels = windows * pixelPerWindow;
inline constexpr quint32 windowByteSize = (colorDepth == 3 || colorDepth == 4)
                                              ? pixelPerWindow * 3 / 2
                                              : pixelPerWindow * 3;
inline constexpr quint32 width = windowPerRow * horizontalPixelUnit;
inline constexpr quint32 height = rows * verticalPixelUnit;
inline constexpr Mode mode = Mode::ROW_WISE;
inline constexpr quint32 maxWindowPerDatagram = 208;
inline constexpr quint32 maxPixelPerDatagram =
    maxWindowPerDatagram * pixelPerWindow;
inline constexpr quint32 packetHeaderSize = 2;
inline constexpr quint32 packetSize =
    maxWindowPerDatagram * windowByteSize + packetHeaderSize;
constexpr quint32 getRemainderPacketSize() {
  quint32 size =
      (windows - maxWindowPerDatagram * (windows / maxWindowPerDatagram)) *
      windowByteSize;

  return (size) ? size + packetHeaderSize : 0;
}
inline constexpr quint32 remainderPacketSize = getRemainderPacketSize();
inline const quint32 maxPacketNumber =
    std::ceil(static_cast<qreal>(windows) / maxWindowPerDatagram);
inline const QImage frame{width, height, QImage::Format_RGB888};
inline constexpr quint32 protocolType = (mode == Mode::ROW_WISE) ? 2 : 1;
inline constexpr quint16 unicastPort = 3000;
inline constexpr quint16 port = 10000;
inline constexpr quint16 commandPort{2000};
inline constexpr quint16 firmwarePort{1997};
inline const QString commandMagic{"SEM"};
inline const QString broadcastAddress{"10.6.255.255"};
inline constexpr quint8 factor = 8 - colorDepth;

// Configuration check
static_assert(rows > 0);
static_assert(roomPerRow > 0);
static_assert(windowPerRoom > 0);
static_assert(verticalPixelUnit > 0);
static_assert(horizontalPixelUnit > 0);
static_assert(colorDepth >= 3 && colorDepth <= 8,
              "Color depth must be between 3 and 8");
static_assert(maxWindowPerDatagram > 0 && maxWindowPerDatagram <= windows);
static_assert(mode == Mode::WINDOW_WISE ||
                  (maxWindowPerDatagram * horizontalPixelUnit *
                   verticalPixelUnit) %
                          (width * verticalPixelUnit) ==
                      0,
              "maxWindowPerDatagram value is not supported in ROW_WISE mode. "
              "Make sure pixels fit into one whole row.");
static_assert(packetHeaderSize > 0);
static_assert(packetSize > 0 && packetSize <= 1472,
              "Packet size must be less than 1472 bytes to avoid IPv4"
              "fragmentation. Assuming 1500 MTU, 1500-20(IPv4 header)-8(UDP "
              "header) = 1472 bytes");
static_assert(mode == Mode::WINDOW_WISE || remainderPacketSize == 0);

}  // namespace defaults

}  // namespace libmueb

#endif  // LIBMUEBCONFIG_H
