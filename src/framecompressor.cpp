#include "framecompressor.h"

#include "libmuebconfig.h"

/* Reference:
 * http://threadlocalmutex.com/?p=48
 * http://threadlocalmutex.com/?page_id=60
 */
namespace {
inline std::uint8_t reduceColor(std::uint8_t c) {
  if (libmueb::defaults::colorDepth == 3)
    return (c * 225 + 4096) >> 13;
  else if (libmueb::defaults::colorDepth == 4)
    return (c * 15 + 135) >> 8;

  return c;
}

void compressColor(QByteArray& datagram, const std::uint8_t* const& frameData,
                   std::uint32_t redIdx, std::uint32_t x) {
  using namespace libmueb::defaults;

  if (colorDepth < 5) {  // < 5 bit color compression
    if (x % 2 == 0) {
      datagram.append(reduceColor(frameData[redIdx]) << factor |
                      reduceColor(frameData[redIdx + 1]));            // RG
      datagram.append(reduceColor(frameData[redIdx + 2]) << factor);  // B
    } else {
      datagram.back() =  // Previous Blue color
          datagram.back() | reduceColor(frameData[redIdx]);  // (B)R
      datagram.append(reduceColor(frameData[redIdx + 1]) << factor |
                      reduceColor(frameData[redIdx + 2]));  // GB
    }
  } else {                                   // 8 bit color not compressed
    datagram.append(frameData[redIdx]);      // R
    datagram.append(frameData[redIdx + 1]);  // G
    datagram.append(frameData[redIdx + 2]);  // B
  }
}
}  // namespace

void FrameCompressor::compressFrame(QImage frame) {
  using namespace libmueb::defaults;

  const auto frameData = frame.constBits();
  qint8 packetNumber = 0;

  QByteArray datagram;
  datagram.reserve(maxWindowPerDatagram * windowByteSize + packetHeaderSize);

  // Packet header
  datagram.append(protocolType);
  datagram.append(packetNumber);

  if (mode == libmueb::Mode::ROW_WISE) {
    for (std::uint32_t pixelIdx = 0; pixelIdx < pixels; ++pixelIdx) {
      compressColor(datagram, frameData, pixelIdx * 3, pixelIdx);

      if ((pixelIdx + 1) % maxPixelPerDatagram == 0) {
        emit datagramReady(datagram);

        datagram.truncate(0);
        datagram.append(protocolType);
        datagram.append(++packetNumber);
      }
    }
  } else if (mode == libmueb::Mode::WINDOW_WISE) {
    for (std::uint32_t windowIdx = 0; windowIdx < windows; ++windowIdx) {
      auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
      auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

      for (std::uint32_t y = 0; y < verticalPixelUnit; ++y) {
        for (std::uint32_t x = 0; x < horizontalPixelUnit * 3; x += 3) {
          auto redIdx = (width * 3) * (row + y) + (col * 3 + x);

          compressColor(datagram, frameData, redIdx, x);
        }
      }

      if ((windowIdx + 1) % maxWindowPerDatagram == 0 ||
          ((windowIdx + 1) == windows && windows % maxWindowPerDatagram != 0)) {
        emit datagramReady(datagram);

        datagram.truncate(0);
        datagram.append(protocolType);
        datagram.append(++packetNumber);
      }
    }
  }
}
