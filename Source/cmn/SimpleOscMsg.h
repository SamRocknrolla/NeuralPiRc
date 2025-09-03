/*
  ==============================================================================

    SimpleOscMsg.h
    Created: 20 Jun 2025 10:32:48am
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <arpa/inet.h>
#else
#define BYTEORDER_FALLBACK
#endif

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>


class SimpleOscMsg {
public:
    class Argument {
    public:
        Argument(int32_t val) : type(INT32), i(val) {}
        Argument(float val) : type(FLOAT32), f(val) {}
        Argument(const std::string& val) : type(STRING), s(val) {}

        Argument(const Argument& other) = default;
        Argument(Argument&& other) noexcept = default;
        Argument& operator=(const Argument& other) = default;
        Argument& operator=(Argument&& other) noexcept = default;
        ~Argument() = default;

        inline bool isInt32() const { return type == INT32; }
        inline bool isFloat32() const { return type == FLOAT32; }
        inline bool isString() const { return type == STRING; }

        inline int32_t getInt32() const { return (isInt32()) ? i : 0; }
        inline float getFloat32() const { return (isFloat32()) ? f : 0.0f; }
        inline std::string getString() const { return (isString()) ? s : std::string{}; }

    private:
        enum Type { INT32, FLOAT32, STRING } type;

        union {
            int32_t i;
            float f;
        };
        std::string s;
    };

    SimpleOscMsg() = default;
    ~SimpleOscMsg() = default;

    SimpleOscMsg(const SimpleOscMsg&) = default;
    SimpleOscMsg(SimpleOscMsg&&) noexcept = default;
    SimpleOscMsg& operator=(const SimpleOscMsg&) = default;
    SimpleOscMsg& operator=(SimpleOscMsg&&) noexcept = default;

    // Construction
    void setAddress(const std::string& addr) { address = addr; }
    void AddInt32(int32_t value) { arguments.emplace_back(value); };
    void AddFloat32(float value) { arguments.emplace_back(value); };
    void AddString(const std::string& value) { arguments.emplace_back(value); };
    void clear() {     arguments.clear(); address.clear(); };

    // JUCE serialization
    void SerializeTo(juce::MemoryBlock& block) const {
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
    };

    bool DeserializeFrom(const juce::MemoryBlock& block, size_t bsize = 0) {
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
    };

    // Getters
    const std::string& getAddress() const { return address; }
    const Argument& operator[](size_t index) const { return arguments[index]; }
    size_t size() const { return arguments.size(); }

    std::vector<Argument>::const_iterator begin() const { return arguments.begin(); }
    std::vector<Argument>::const_iterator end() const { return arguments.end(); }

private:
    std::string address;
    std::vector<Argument> arguments;

    static size_t readPaddedString(const uint8_t* data, size_t offset, std::string& out) {
        size_t start = offset;
        while (data[offset] != '\0') ++offset;
        out.assign(reinterpret_cast<const char*>(data + start), offset - start);
        return align4(offset + 1);
    };
    static void writePaddedString(std::vector<uint8_t>& buffer, const std::string& str) {
        buffer.insert(buffer.end(), str.begin(), str.end());
        buffer.push_back('\0');
        while (buffer.size() % 4 != 0)
             buffer.push_back('\0');
    };
    static inline size_t align4(size_t n) { return (n + 3) & ~3; }
};