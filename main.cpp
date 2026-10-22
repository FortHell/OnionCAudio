#include <iostream>
#include <cstdint>
#include <fstream>
#include <type_traits>

#pragma pack(push, 1)
struct OCAudioHeader {
    std::uint8_t val_id[4] = {};
    std::uint8_t version_major = 255;
    std::uint8_t version_minor = 255;
    std::uint16_t volume_offset = 0;

    std::uint32_t sample_rate = 0;
    std::uint16_t stem_count = 0;
    std::uint8_t fadein_sec = 0;
    std::uint8_t fadeout_sec = 0;
    std::uint32_t block_size = 0;

    std::uint32_t total_frames = 0;
    std::uint32_t loop_start = 0;
    std::uint32_t loop_end = 0;

    std::uint64_t reserved[4] = {};
};
#pragma pack(pop)

static_assert(sizeof(OCAudioHeader) == 64);
static_assert(std::is_trivially_copyable_v<OCAudioHeader>);

int outputFile(const OCAudioHeader& m_header) {
    std::ofstream f("test.ocaudio", std::ios::binary);

    if (!f.is_open()) {
        std::cerr << "Error writing to file! (100)\n";
        return 1;
    }

    f.write(reinterpret_cast<const char*>(&m_header), sizeof(m_header));

    if (!f) {
        std::cerr << "Write failed! (101)\n";
        return 1;
    }

    return 0;
}

int main() {
    OCAudioHeader header;

    header.val_id[0] = 'O';
    header.val_id[1] = 'C';
    header.val_id[2] = 'A';
    header.val_id[3] = '_';

    header.version_major = 0;
    header.version_minor = 1;

    header.sample_rate = 48000;
    header.stem_count = 4;
    header.block_size = 128;

    std::cout << "Size of header: " << sizeof(header) << " bytes\n";
    static_assert(sizeof(header) == 64, "Incorrect header size! (102)");

    outputFile(header);
    
    return 0;
}