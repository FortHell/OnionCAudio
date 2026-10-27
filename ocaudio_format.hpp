#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// ocaudio_format header

// Needs to be little-endian
static_assert(std::endian::native == std::endian::little, "Must be little-endian. Byte-swapping for big-endian hosts.");

namespace ocaudio {

constexpr std::uint8_t  kVersionMajor   = 0;
constexpr std::uint8_t  kVersionMinor   = 2;
constexpr std::uint16_t kHeaderSize     = 64;
// 15 stereo stems = 30 individual QOA channels
constexpr std::uint16_t kMaxStemsV1     = 15;
constexpr std::uint8_t  kChannelsPerStem = 2;

// Q4.12 fixed-point multiplier: 4096 == 1.0
constexpr std::uint16_t kVolumeOne = 4096;
constexpr std::uint16_t kVolumeMax = 10 * kVolumeOne;

enum class Codec : std::uint8_t {
    None = 0,
    QOA  = 1,
};

enum class OutputFormat : std::uint8_t {
    Float32 = 1,
};

enum Flags : std::uint8_t {
    FlagLoop = 1 << 0,
};

enum class Error : int {
    Ok = 0,
    OpenFailed = 100,
    WriteFailed = 101,
    ReadFailed = 102,
    BadMagic = 103,
    BadVersion = 104,
    BadHeaderSize = 105,
    BadStemCount = 106,
    BadChannels = 107,
    BadCodec = 108,
    BadOutputFormat = 109,
    BadVolume = 110,
    BadLoop = 111,
    BadSampleRate = 112,
    ReservedNotZero = 113,
};

#pragma pack(push, 1)
struct Header {
    char          magic[4]          = {'O', 'C', 'A', '_'};
    std::uint8_t  version_major     = kVersionMajor;
    std::uint8_t  version_minor     = kVersionMinor;
    std::uint16_t header_size       = kHeaderSize;

    std::uint32_t sample_rate       = 0;
    std::uint32_t total_frames      = 0;

    std::uint16_t stem_count        = 0;
    std::uint8_t  channels_per_stem = kChannelsPerStem;
    std::uint8_t  codec             = static_cast<std::uint8_t>(Codec::QOA);
    std::uint8_t  output_format     = static_cast<std::uint8_t>(OutputFormat::Float32);
    std::uint8_t  flags             = 0;

    std::uint16_t fadein_ms         = 0;
    std::uint16_t fadeout_ms        = 0;
    std::uint16_t volume_q4_12      = kVolumeOne;

    std::uint32_t loop_start        = 0;     // defined in frames
    std::uint32_t loop_end          = 0;     // defined in frames (exclusive)
    std::uint32_t index_offset      = 0;     // seek table, 0 == none
    std::uint32_t data_offset       = 0;     // start of QOA stream
    std::uint32_t crc32             = 0;     // 0 == unused

    std::uint8_t  reserved[16]      = {};
};
#pragma pack(pop)

static_assert(sizeof(Header) == kHeaderSize, "Incorrect header size!");
static_assert(std::is_trivially_copyable_v<Header>);
static_assert(offsetof(Header, header_size)  == 6);
static_assert(offsetof(Header, sample_rate)  == 8);
static_assert(offsetof(Header, total_frames) == 12);
static_assert(offsetof(Header, stem_count)   == 16);
static_assert(offsetof(Header, flags)        == 21);
static_assert(offsetof(Header, fadein_ms)    == 22);
static_assert(offsetof(Header, volume_q4_12) == 26);
static_assert(offsetof(Header, loop_start)   == 28);
static_assert(offsetof(Header, index_offset) == 36);
static_assert(offsetof(Header, data_offset)  == 40);
static_assert(offsetof(Header, crc32)        == 44);
static_assert(offsetof(Header, reserved)     == 48);

// Converts a float multiplier to Q4.12, clamped to [0, 10].
inline std::uint16_t volumeToQ4_12(float v) {
    if (!(v > 0.0f)) return 0;   // also catches NaN
    if (v >= 10.0f) return kVolumeMax;
    return static_cast<std::uint16_t>(v * kVolumeOne + 0.5f);
}

inline float volumeFromQ4_12(std::uint16_t raw) {
    return static_cast<float>(raw) / static_cast<float>(kVolumeOne);
}

// Error checking
inline Error validateHeader(const Header& h) {
    if (h.magic[0] != 'O' || h.magic[1] != 'C' || h.magic[2] != 'A' || h.magic[3] != '_')
        return Error::BadMagic;
    if (h.version_major != kVersionMajor || h.version_minor != kVersionMinor)
        return Error::BadVersion;
    if (h.header_size != kHeaderSize)
        return Error::BadHeaderSize;
    // QOA stores the sample rate in 24 bits and requires >= 1.
    if (h.sample_rate == 0 || h.sample_rate > 0xFFFFFF)
        return Error::BadSampleRate;
    if (h.stem_count == 0 || h.stem_count > kMaxStemsV1)
        return Error::BadStemCount;
    if (h.channels_per_stem != kChannelsPerStem)
        return Error::BadChannels;
    if (h.codec != static_cast<std::uint8_t>(Codec::None) &&
        h.codec != static_cast<std::uint8_t>(Codec::QOA))
        return Error::BadCodec;
    if (h.output_format != static_cast<std::uint8_t>(OutputFormat::Float32))
        return Error::BadOutputFormat;
    if (h.volume_q4_12 > kVolumeMax)
        return Error::BadVolume;
    if (h.flags & FlagLoop) {
        if (h.loop_start >= h.loop_end || h.loop_end > h.total_frames)
            return Error::BadLoop;
    }
    for (std::uint8_t b : h.reserved) {
        if (b != 0) return Error::ReservedNotZero;
    }
    return Error::Ok;
}

}
