#pragma once

#include <QImage>
#include <QString>
#include <cmath>
#include <cstdint>

namespace libmueb {

enum class Mode { WINDOW_WISE, ROW_WISE };

namespace defaults {

// Hardware specific constants
inline constexpr std::uint32_t rows{13};
inline constexpr std::uint32_t roomPerRow{8};
inline constexpr std::uint32_t windowPerRoom{2};
inline constexpr std::uint32_t verticalPixelUnit{2};
inline constexpr std::uint32_t horizontalPixelUnit{2};
inline constexpr std::uint8_t colorDepth{3};

// Debug specific constants
// Send packets to localhost
inline constexpr bool debugMode{false};

// Software specific constants
inline constexpr std::uint32_t pixelPerWindow{verticalPixelUnit *
                                              horizontalPixelUnit};
inline constexpr std::uint32_t windowPerRow{roomPerRow * windowPerRoom};
inline constexpr std::uint32_t windows{rows * windowPerRow};
inline constexpr std::uint32_t pixels{windows * pixelPerWindow};
inline constexpr std::uint32_t width{windowPerRow * horizontalPixelUnit};
inline constexpr std::uint32_t height{rows * verticalPixelUnit};
inline constexpr std::uint8_t factor{4};

// Network protocol specific constants
inline constexpr Mode mode{Mode::ROW_WISE};
inline constexpr std::uint32_t protocolType{(mode == Mode::ROW_WISE) ? 2 : 1};
inline constexpr std::uint16_t port{50001};
inline constexpr std::uint16_t commandPort{50000};
inline constexpr std::uint16_t firmwarePort{50002};
inline constexpr const char* const commandMagic{"SEM"};
inline constexpr const char* const broadcastAddress{
    (debugMode) ? "127.0.0.1" : "10.6.255.255"};
inline constexpr std::uint32_t windowByteSize =
    (colorDepth == 3 || colorDepth == 4) ? pixelPerWindow * 3 / 2
                                         : pixelPerWindow * 3;
inline constexpr std::uint32_t maxWindowPerDatagram{windows};
inline constexpr std::uint32_t packetHeaderSize{2};
inline constexpr std::uint32_t packetSize{
    maxWindowPerDatagram * windowByteSize + packetHeaderSize};
inline constexpr std::uint32_t maxPixelPerDatagram{maxWindowPerDatagram *
                                                   pixelPerWindow};

inline constexpr std::uint32_t getRemainderPacketSize() {
  std::uint32_t size{
      (windows - maxWindowPerDatagram * (windows / maxWindowPerDatagram)) *
      windowByteSize};

  return (size) ? size + packetHeaderSize : 0;
}

inline constexpr std::uint32_t remainderPacketSize{getRemainderPacketSize()};
inline const std::uint32_t maxPacketNumber{static_cast<std::uint32_t>(
    std::ceil(static_cast<float>(windows) / maxWindowPerDatagram))};

// Default constructed objects
inline const QImage frame{width, height, QImage::Format_RGB888};

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
