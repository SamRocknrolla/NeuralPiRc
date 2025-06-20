/*
  ==============================================================================

    SimpleOscMsg.h
    Created: 20 Jun 2025 10:32:48am
    Author:  Sam

  ==============================================================================
*/

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <juce_core/juce_core.h>

class SimpleOscMsg {
public:
    struct Argument {
        enum Type { INT32, FLOAT32, STRING } type;

        union {
            int32_t i;
            float f;
        };
        std::string s;

        Argument(int32_t val) : type(INT32), i(val) {}
        Argument(float val) : type(FLOAT32), f(val) {}
        Argument(const std::string& val) : type(STRING), s(val) {}

        Argument(const Argument& other) = default;
        Argument(Argument&& other) noexcept = default;
        Argument& operator=(const Argument& other) = default;
        Argument& operator=(Argument&& other) noexcept = default;
        ~Argument() = default;
    };

    SimpleOscMsg() = default;
    ~SimpleOscMsg() = default;

    SimpleOscMsg(const SimpleOscMsg&) = default;
    SimpleOscMsg(SimpleOscMsg&&) noexcept = default;
    SimpleOscMsg& operator=(const SimpleOscMsg&) = default;
    SimpleOscMsg& operator=(SimpleOscMsg&&) noexcept = default;

    // Construction
    void setAddress(const std::string& addr) { address = addr; }
    void AddInt32(int32_t value);
    void AddFloat32(float value);
    void AddString(const std::string& value);

    // JUCE serialization
    void SerializeTo(juce::MemoryBlock& block);
    bool DeserializeFrom(const juce::MemoryBlock& block);

    // Getters
    const std::string& getAddress() const { return address; }
    const Argument& operator[](size_t index) const { return arguments[index]; }
    size_t size() const { return arguments.size(); }

    std::vector<Argument>::const_iterator begin() const { return arguments.begin(); }
    std::vector<Argument>::const_iterator end() const { return arguments.end(); }

private:
    std::string address;
    std::vector<Argument> arguments;

    size_t readPaddedString(const uint8_t* data, size_t offset, std::string& out);
    void writePaddedString(std::vector<uint8_t>& buffer, const std::string& str);
    size_t align4(size_t n) const { return (n + 3) & ~3; }
};