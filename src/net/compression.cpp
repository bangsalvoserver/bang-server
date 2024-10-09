#include "compression.h"

#include <stdexcept>

#include <zlib.h>

#include "utils/defer.h"

namespace compression {

    static constexpr size_t initial_buffer_size = 4096;
    
    std::vector<uint8_t> compress_bytes(std::span<const uint8_t> bytes) {
        z_stream strm{};

        if (deflateInit(&strm, Z_BEST_COMPRESSION) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib for compression");
        }

        defer { deflateEnd(&strm); };

        strm.avail_in = bytes.size();
        strm.next_in = const_cast<uint8_t*>(bytes.data());

        std::vector<uint8_t> output_bytes;
        output_bytes.resize(initial_buffer_size);

        size_t bytes_written = 0;

        int result;
        do {
            if (bytes_written >= output_bytes.size()) {
                output_bytes.resize(output_bytes.size() * 2);
            }
            size_t bytes_to_write = output_bytes.size() - bytes_written;

            strm.avail_out = bytes_to_write;
            strm.next_out = output_bytes.data() + bytes_written;

            result = deflate(&strm, Z_FINISH);

            if (result != Z_STREAM_END && result != Z_OK) {
                throw std::runtime_error("Compression failed");
            }

            bytes_written += bytes_to_write - strm.avail_out;
        } while (result != Z_STREAM_END);

        output_bytes.resize(bytes_written);
        return output_bytes;
    }

    std::vector<uint8_t> decompress_bytes(std::span<const uint8_t> bytes) {
        z_stream strm{};

        if (inflateInit(&strm) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib for decompression");
        }

        defer { inflateEnd(&strm); };

        strm.avail_in = bytes.size();
        strm.next_in = const_cast<uint8_t*>(bytes.data());

        std::vector<uint8_t> output_bytes;
        output_bytes.resize(initial_buffer_size);

        size_t bytes_written = 0;

        int result;
        do {
            if (bytes_written >= output_bytes.size()) {
                output_bytes.resize(output_bytes.size() * 2);
            }
            size_t bytes_to_write = output_bytes.size() - bytes_written;

            strm.avail_out = bytes_to_write;
            strm.next_out = output_bytes.data() + bytes_written;

            result = inflate(&strm, Z_NO_FLUSH);

            if (result != Z_STREAM_END && result != Z_OK) {
                throw std::runtime_error("Decompression failed");
            }

            bytes_written += bytes_to_write - strm.avail_out;
        } while (result != Z_STREAM_END);

        output_bytes.resize(bytes_written);
        return output_bytes;
    }
    
}