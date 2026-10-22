#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

#include "ocaudio_format.hpp"

using ocaudio::Error;
using ocaudio::Header;

Error writeHeader(const char* path, const Header& header) {
    std::ofstream f(path, std::ios::binary);

    if (!f.is_open()) {
        std::cerr << "Error opening file for writing! (" << static_cast<int>(Error::OpenFailed) << ")\n";
        return Error::OpenFailed;
    }

    f.write(reinterpret_cast<const char*>(&header), sizeof(header));

    if (!f) {
        std::cerr << "Write failed! (" << static_cast<int>(Error::WriteFailed) << ")\n";
        return Error::WriteFailed;
    }

    return Error::Ok;
}

Error readHeader(const char* path, Header& header) {
    std::ifstream f(path, std::ios::binary);

    if (!f.is_open()) {
        std::cerr << "Error opening file for reading! (" << static_cast<int>(Error::OpenFailed) << ")\n";
        return Error::OpenFailed;
    }

    f.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!f) {
        std::cerr << "Read failed! (" << static_cast<int>(Error::ReadFailed) << ")\n";
        return Error::ReadFailed;
    }

    return Error::Ok;
}

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "test.ocaudio";

    Header header;
    header.sample_rate = 48000;
    header.stem_count = 4;
    header.volume_q4_12 = ocaudio::volumeToQ4_12(1.0f);

    std::cout << "Size of header: " << sizeof(header) << " bytes\n";

    Error err = ocaudio::validateHeader(header);
    if (err != Error::Ok) {
        std::cerr << "Invalid header! (" << static_cast<int>(err) << ")\n";
        return static_cast<int>(err);
    }

    err = writeHeader(path, header);
    if (err != Error::Ok) return static_cast<int>(err);

    // Read back and verify round trip.
    Header readBack;
    err = readHeader(path, readBack);
    if (err != Error::Ok) return static_cast<int>(err);

    err = ocaudio::validateHeader(readBack);
    if (err != Error::Ok) {
        std::cerr << "Read-back header invalid! (" << static_cast<int>(err) << ")\n";
        return static_cast<int>(err);
    }

    if (std::memcmp(&header, &readBack, sizeof(Header)) != 0) {
        std::cerr << "Round-trip mismatch!\n";
        return static_cast<int>(Error::ReadFailed);
    }

    std::cout << "Wrote and verified " << path << " (volume x"
              << ocaudio::volumeFromQ4_12(readBack.volume_q4_12) << ")\n";
    return 0;
}
