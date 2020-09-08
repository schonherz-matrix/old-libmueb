#include "datagramprocessor.h"

#include <QDebug>

namespace {
void uncompressColor(const QByteArray& data, std::uint8_t* const& frameData,
                     const std::uint32_t& frameIdx, std::uint32_t& dataIdx,
                     const std::uint32_t& x) {
  using namespace libmueb::defaults;

  if (colorDepth < 5) {
    if (x % 2 == 0) {
      frameData[frameIdx] = data[dataIdx] & 0xf0;                  // R(G)
      frameData[frameIdx + 1] = (data[dataIdx] & 0x0f) << factor;  // (R)G
      frameData[frameIdx + 2] = data[dataIdx + 1] & 0xf0;          // B(R)

      dataIdx++;
    } else {
      frameData[frameIdx] = (data[dataIdx] & 0x0f) << factor;          // (B)R
      frameData[frameIdx + 1] = data[dataIdx + 1] & 0xf0;              // G(B)
      frameData[frameIdx + 2] = (data[dataIdx + 1] & 0x0f) << factor;  // (G)B

      dataIdx += 2;
    }
  } else {
    frameData[frameIdx] = data[dataIdx];          // R
    frameData[frameIdx + 1] = data[dataIdx + 1];  // G
    frameData[frameIdx + 2] = data[dataIdx + 2];  // B

    dataIdx += 3;
  }
}

inline void datagramUncompressError() {
  qWarning() << "[MuebReceiver] Processed packet is invalid! Check the header "
                "or packet contents(size)";
}
}  // namespace

DatagramProcessor::DatagramProcessor() { m_frame.fill(Qt::black); }

QImage DatagramProcessor::frame() const { return m_frame; }

void DatagramProcessor::processDatagram(const QByteArray datagram) {
  using namespace libmueb::defaults;

  // Packet header check
  auto protocol = datagram[0];
  if (protocol != 1 && protocol != 2) {
    datagramUncompressError();
    return;
  }

  const std::uint32_t packetNumber = datagram[1];
  if (packetNumber >= maxPacketNumber || packetNumber < 0) {
    datagramUncompressError();
    return;
  }

  const auto frameData = m_frame.bits();
  auto dataIdx = packetHeaderSize;

  if (protocol == 2) {
    for (std::uint32_t pixelIdx = packetNumber * maxPixelPerDatagram;
         pixelIdx < (packetNumber + 1) * maxPixelPerDatagram; ++pixelIdx) {
      // Check datagram index
      // Drop invalid packet
      if (dataIdx >= datagram.size()) {
        datagramUncompressError();
        return;
      }

      auto frameIdx = pixelIdx * 3;
      uncompressColor(datagram, frameData, frameIdx, dataIdx, pixelIdx);
    }
  } else if (protocol == 1) {
    for (std::uint32_t windowIdx = packetNumber * maxWindowPerDatagram;
         windowIdx < (packetNumber + 1) * maxWindowPerDatagram &&
         windowIdx < windows;
         ++windowIdx) {
      auto row = (windowIdx / windowPerRow) * verticalPixelUnit;
      auto col = (windowIdx % windowPerRow) * horizontalPixelUnit;

      for (std::uint32_t y = 0; y < verticalPixelUnit; ++y) {
        for (std::uint32_t x = 0; x < horizontalPixelUnit * 3; x += 3) {
          // Check datagram index
          // Drop invalid packet
          if (dataIdx >= datagram.size()) {
            datagramUncompressError();
            return;
          }

          auto frameIdx = (m_frame.width() * 3) * (row + y) + (col * 3 + x);

          uncompressColor(datagram, frameData, frameIdx, dataIdx, x);
        }
      }
    }
  }

  emit frameReady(m_frame);
}
