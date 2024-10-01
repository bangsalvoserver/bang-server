#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <vector>
#include <cstddef>
#include <stdexcept>

#include <zlib.h>

namespace compression {
    
    inline std::vector<std::byte> compress_bytes(const std::vector<std::byte> &bytes) {
        size_t compressed_size = compressBound(bytes.size());
        std::vector<std::byte> buffer{compressed_size};

        int result = compress(
            reinterpret_cast<unsigned char *>(buffer.data()), &compressed_size,
            reinterpret_cast<const unsigned char *>(bytes.data()), bytes.size()
        );

        if (result != Z_OK) {
            throw std::runtime_error("Could not compress bytes");
        }

        buffer.resize(compressed_size);
        return buffer;
    }

    inline std::vector<std::byte> decompress_bytes(const std::vector<std::byte> &bytes, size_t uncompressed_size) {
        std::vector<std::byte> buffer{uncompressed_size};

        int result = uncompress(
            reinterpret_cast<unsigned char *>(buffer.data()), &uncompressed_size,
            reinterpret_cast<const unsigned char *>(bytes.data()), bytes.size()
        );

        if (result != Z_OK) {
            throw std::runtime_error("Could not decompress bytes");
        }

        buffer.resize(uncompressed_size);
        return buffer;
    }
    
}

#endif