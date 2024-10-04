#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <vector>
#include <cstddef>

namespace compression {
    
    std::vector<std::byte> compress_bytes(const std::vector<std::byte> &inputData);

    std::vector<std::byte> decompress_bytes(const std::vector<std::byte> &compressedData);
    
}

#endif