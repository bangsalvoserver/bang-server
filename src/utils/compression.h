#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <vector>
#include <cstddef>
#include <stdexcept>

#include <zlib.h>

namespace compression {
    
    inline std::vector<std::byte> compress_bytes(const std::vector<std::byte> &bytes) {
        uLong bufsize = compressBound(bytes.size());
        std::vector<std::byte> buffer{bufsize};

        int result = compress(
            reinterpret_cast<Bytef *>(buffer.data()), &bufsize,
            reinterpret_cast<const Bytef *>(bytes.data()), bytes.size()
        );

        if (result != Z_OK) {
            throw std::runtime_error("Could not compress bytes");
        }

        buffer.resize(bufsize);
        return buffer;
    }

    inline std::vector<std::byte> decompress_bytes(const std::vector<std::byte> &bytes, size_t uncompressed_size) {
        std::vector<std::byte> buffer{uncompressed_size};
        uLong bufsize = uncompressed_size;

        int result = uncompress(
            reinterpret_cast<Bytef *>(buffer.data()), &bufsize,
            reinterpret_cast<const Bytef *>(bytes.data()), bytes.size()
        );

        if (result != Z_OK) {
            throw std::runtime_error("Could not decompress bytes");
        }

        buffer.resize(bufsize);
        return buffer;
    }
    
}

#endif