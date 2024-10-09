#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <span>
#include <vector>
#include <cstdint>

namespace compression {
    
    std::vector<uint8_t> compress_bytes(std::span<const uint8_t> bytes);

    std::vector<uint8_t> decompress_bytes(std::span<const uint8_t> bytes);
    
}

#endif