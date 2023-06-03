#ifndef FBXPROPERTY_H
#define FBXPROPERTY_H

#include <memory>
#include <iostream>
#include <vector>

namespace fbx 
{

    
// WARNING: (copied from fbxutil.h)
// this assumes that float is 32bit and double is 64bit
// both conforming to IEEE 754, it does not assume endianness
// it also assumes that signed integers are two's complement
union FBXPropertyValue {
    int16_t i16;  // Y
    bool boolean; // C, b
    int32_t i32;  // I, i
    float f32;    // F, f
    double f64;   // D, d
    int64_t i64;  // L, l
};

class FBXProperty
{
public:
    enum Type : uint8_t
    {
        SHORT = 'Y',    //!< i16, 'Y'
        BOOLEAN = 'C',  //!< 1 byte, 'C'
        LONG = 'L',     //!< 8 bytes, 'L'
        INTEGER = 'I',  //!< 4 bytes, 'I'
        STRING = 'S',   //!< string, 'S'
        STRING2 = 'R',  //!< string, 'R'
        FLOAT = 'F',    //!< 4 bytes float, 'F'
        DOUBLE = 'D',   //!< 8 bytes double, 'D'
        ARRAY_BOOLEAN = 'b',    //!< array of 1 byte bools, 'b'
        ARRAY_DOUBLE = 'd',     //!< array of 8 bytes double, 'd'
        ARRAY_INT = 'i',        //!< array of 4 bytes int, 'i'
        ARRAY_LONG = 'l',       //!< array of 8 bytes long, 'l'
        ARRAY_FLOAT = 'f'       //!< array of 4 bytes float, 'f'
    };

    FBXProperty(std::ifstream &input);
    // primitive values
    FBXProperty(int16_t);
    FBXProperty(bool);
    FBXProperty(int32_t);
	FBXProperty(uint32_t);
    FBXProperty(float);
    FBXProperty(double);
    FBXProperty(int64_t);
    // arrays
    FBXProperty(const std::vector<bool>&);
    FBXProperty(const std::vector<int32_t>&);
    FBXProperty(const std::vector<float>&);
    FBXProperty(const std::vector<double>&);
    FBXProperty(const std::vector<int64_t>&);
    // raw / string
    FBXProperty(const std::vector<uint8_t>&, uint8_t type);
    FBXProperty(const std::string&);
    FBXProperty(const char *);

    void write(std::ofstream &output);

	// json format
    std::string to_string(bool skip_quotes=false, bool exit_on_zero_char=false) const;
	// fbx ascii format
	std::string to_ascii(uint32_t tab_offset) const;
    
    Type GetType() const;
    void GetData(void* buffer) const;

    int AsInt() const { return value.i32; }
    int64_t AsLong() const { return value.i64; }
    double AsDouble() const { return value.f64; }
    float AsFloat() const { return value.f32; }

    bool is_array();
    
    //!< number of bytes required for the property data
    uint32_t GetBytesCount() const;
    size_t GetCount() const;
private:
    Type type;
    FBXPropertyValue value;
    std::vector<uint8_t> raw;
    std::vector<FBXPropertyValue> values;
};

} // namespace fbx

#endif // FBXPROPERTY_H
