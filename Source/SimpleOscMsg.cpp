/*
  ==============================================================================

    SimpleOscMsg.cpp
    Created: 20 Jun 2025 10:32:48am
    Author:  Sam

  ==============================================================================
*/

#include "SimpleOscMsg.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <arpa/inet.h>
#else
#define BYTEORDER_FALLBACK
#endif

void SimpleOscMsg::AddInt32(int32_t value) {
    arguments.emplace_back(value);
}

void SimpleOscMsg::AddFloat32(float value) {
    arguments.emplace_back(value);
}

void SimpleOscMsg::AddString(const std::string& value) {
    arguments.emplace_back(value);
}

void SimpleOscMsg::clear() {
    arguments.clear();
    address.clear();
}

void SimpleOscMsg::SerializeTo(juce::MemoryBlock& block) const {
    std::vector<uint8_t> outBuffer;

    // Write address
    writePaddedString(outBuffer, address);

    // Type tags
    std::string tag = ",";
    for (const auto& arg : arguments) {
        if (arg.isInt32()) 
            tag += 'i';
        else if (arg.isFloat32()) 
            tag += 'f';
        else if (arg.isString()) 
            tag += 's';
    }
    writePaddedString(outBuffer, tag);

    // Write argument data
    for (const auto& arg : arguments) {
        if (arg.isInt32()) {
            int32_t net = htonl(arg.getInt32());
            const uint8_t* p = reinterpret_cast<const uint8_t*>(&net);
            outBuffer.insert(outBuffer.end(), p, p + 4);
        }
        else if (arg.isFloat32()) {
            float temp = arg.getFloat32();
            int32_t* net = reinterpret_cast<int32_t*>(&temp);
            *net = htonl(*net);
            const uint8_t* p = reinterpret_cast<const uint8_t*>(net);
            outBuffer.insert(outBuffer.end(), p, p + 4);
        }
        else if (arg.isString()) {
            writePaddedString(outBuffer, arg.getString());
        }
    }

    block.setSize(outBuffer.size(), false);
    std::memcpy(block.getData(), outBuffer.data(), outBuffer.size());
}

bool SimpleOscMsg::DeserializeFrom(const juce::MemoryBlock& block, size_t bsize) {
    arguments.clear();
    const uint8_t* data = static_cast<const uint8_t*>(block.getData());
    size_t size = (bsize)? bsize : block.getSize();
    size_t offset = 0;

    // Address
    offset = readPaddedString(data, offset, address);
    if (offset >= size) return false;

    // Type tag string
    std::string typeTags;
    offset = readPaddedString(data, offset, typeTags);
    if (typeTags.empty() || typeTags[0] != ',') return false;

    for (size_t i = 1; i < typeTags.size(); ++i) {
        if (offset + 4 > size) return false;

        switch (typeTags[i]) {
        case 'i': {
            int32_t val;
            std::memcpy(&val, data + offset, 4);
            val = ntohl(val);
            arguments.emplace_back(val);
            offset += 4;
            break;
        }
        case 'f': {
            uint32_t temp;
            std::memcpy(&temp, data + offset, 4);
            temp = ntohl(temp);
            float val;
            std::memcpy(&val, &temp, 4);
            arguments.emplace_back(val);
            offset += 4;
            break;
        }
        case 's': {
            std::string str;
            offset = readPaddedString(data, offset, str);
            arguments.emplace_back(str);
            break;
        }
        default:
            return false;
        }
    }

    return true;
}

size_t SimpleOscMsg::readPaddedString(const uint8_t* data, size_t offset, std::string& out) {
    size_t start = offset;
    while (data[offset] != '\0') ++offset;
    out.assign(reinterpret_cast<const char*>(data + start), offset - start);
    return align4(offset + 1);
}

void SimpleOscMsg::writePaddedString(std::vector<uint8_t>& buffer, const std::string& str) {
    buffer.insert(buffer.end(), str.begin(), str.end());
    buffer.push_back('\0');
    while (buffer.size() % 4 != 0)
        buffer.push_back('\0');
}