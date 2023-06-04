#include "fbxproperty.h"
#include "fbxutil.h"
#include <functional>
#include "miniz.h"

using std::cout;
using std::endl;
using std::string;

namespace fbx {

namespace { // helpers for reading properties
    FBXPropertyValue ReadPrimitiveValue(Reader &reader, char type)
    {
        FBXPropertyValue value;
        if(type == 'Y') { // 2 byte signed integer
            value.i16 = reader.readInt16();
        } else if(type == 'C' || type == 'B') { // 1 bit boolean (1: true, 0: false) encoded as the LSB of a 1 Byte value.
            value.boolean = reader.readUint8() != 0;
        } else if(type == 'I') { // 4 byte signed Integer
            value.i32 = reader.readInt32();
        } else if(type == 'F') { // 4 byte single-precision IEEE 754 number
            value.f32 = reader.readFloat();
        } else if(type == 'D') { // 8 byte double-precision IEEE 754 number
            value.f64 = reader.readDouble();
        } else if(type == 'L') { // 8 byte signed Integer
            value.i64 = reader.readUint64();
        } else {
            throw std::string("Unsupported property type ")+std::to_string(type);
        }
        return value;
    }

    uint32_t ArrayElementSize(char type)
    {
        if(type == 'Y') { // 2 byte signed integer
            return 2;
        } else if(type == 'C' || type == 'B') { // 1 bit boolean (1: true, 0: false) encoded as the LSB of a 1 Byte value.
            return 1;
        } else if(type == 'I') { // 4 byte signed Integer
            return 4;
        } else if(type == 'F') { // 4 byte single-precision IEEE 754 number
            return 4;
        } else if(type == 'D') { // 8 byte double-precision IEEE 754 number
            return 8;
        } else if(type == 'L') { // 8 byte signed Integer
            return 8;
        } else {
            return 0;
        }
    }

    class STRMAutoCloser
    {
    public:
        z_stream *strm;
        STRMAutoCloser(z_stream& _strm):strm(&_strm) {}

        ~STRMAutoCloser() {
            (void)inflateEnd(strm);
        }
    };

    class BufferAutoFree
    {
    public:
        uint8_t *buffer;
        BufferAutoFree(uint8_t *buf):buffer(buf) {}

        ~BufferAutoFree() {
            free(buffer);
        }
    };
}

FBXProperty::FBXProperty(std::ifstream &input)
{
    Reader reader(&input);
    type = static_cast<Type>(reader.readUint8());
    // std::cout << "  " << type << "\n";
    if(type == Type::STRING || type == Type::STRING2) 
    {    
		const uint32_t length = reader.readUint32();
		raw.resize(length);

		for (auto iter = begin(raw); iter != end(raw); ++iter)
		{
			*iter = reader.readUint8();
		}

    } 
    else if(type < 'Z') 
    { // primitive types
        value = ReadPrimitiveValue(reader, type);
    } 
    else 
    {
        const uint32_t arrayLength = reader.readUint32(); // number of elements in array
        const uint32_t encoding = reader.readUint32(); // 0 .. uncompressed, 1 .. zlib-compressed
        const uint32_t compressedLength = reader.readUint32();
        if(encoding) {
            uint64_t uncompressedLength = ArrayElementSize(type - ('a' - 'A')) * static_cast<uint64_t>(arrayLength);

            uint8_t *decompressedBuffer = (uint8_t*) malloc( (size_t) uncompressedLength);
            if(decompressedBuffer == nullptr) throw std::string("Malloc failed");
            BufferAutoFree baf(decompressedBuffer);

            uint8_t *compressedBuffer = new uint8_t[compressedLength];
            reader.read((char*)compressedBuffer, compressedLength);

            uint64_t destLen = uncompressedLength;
            uint64_t srcLen = compressedLength;
			// uncompress2

			mz_ulong mz_destLen = (mz_ulong) destLen;
			mz_ulong mz_srcLen = (mz_ulong) srcLen;

			uncompress(decompressedBuffer, &mz_destLen, compressedBuffer, mz_srcLen);
			destLen = mz_destLen;

			if (nullptr != compressedBuffer)
			{
				delete[] compressedBuffer;
				compressedBuffer = nullptr;
			}

            if(srcLen != compressedLength) throw std::string("compressedLength does not match data");
            if(destLen != uncompressedLength) throw std::string("uncompressedLength does not match data");

            Reader r((char*)decompressedBuffer);

            for(uint32_t i = 0; i < arrayLength; i++) {
                values.push_back(ReadPrimitiveValue(r, type - ('a'-'A')));
            }
        } else {

			values.resize(arrayLength);

			for (auto iter = begin(values); iter != end(values); ++iter)
			{
				*iter = ReadPrimitiveValue(reader, type - ('a' - 'A'));
			}
        }
    }
}

void FBXProperty::write(std::ostream* output)
{
    Writer writer(output);

    writer.write(type);

    switch (type)
    {
    case Type::SHORT: writer.write(value.i16); break;
    case Type::BOOLEAN: writer.write(static_cast<uint8_t>(value.boolean ? 1 : 0)); break;
    case Type::INTEGER: writer.write(value.i32); break;
    case Type::FLOAT: writer.write(value.f32); break;
    case Type::DOUBLE: writer.write(value.f64); break;
    case Type::LONG: writer.write(value.i64); break;
    case Type::STRING:
    case Type::STRING2:
        {
            writer.write(static_cast<uint32_t>(raw.size()));
            for (size_t i = 0, count = raw.size(); i < count; ++i)
                writer.write(static_cast<uint8_t>(raw[i]));
        } break;
    default:

        writer.write(static_cast<uint32_t>(values.size())); // arrayLength
        writer.write(static_cast<uint32_t>(0)); // encoding // TODO: support compression
        uint32_t compressedLength = static_cast<uint32_t>(values.size());

        switch (type)
        {
        case Type::ARRAY_FLOAT:
        case Type::ARRAY_INT: 
            compressedLength *= 4;
            break;
        case Type::ARRAY_DOUBLE:
        case Type::ARRAY_LONG:
            compressedLength *= 8;
            break;
        case Type::ARRAY_BOOLEAN:
            break;
        default:
            throw std::string("Invalid property");
        }

        writer.write(compressedLength);

        for (auto& e : values) 
        {
            switch (type)
            {
            case Type::ARRAY_FLOAT: writer.write(e.f32); break;
            case Type::ARRAY_DOUBLE: writer.write(e.f64); break;
            case Type::ARRAY_LONG: writer.write(static_cast<int64_t>(e.i64)); break;
            case Type::ARRAY_INT: writer.write(e.i32); break;
            case Type::ARRAY_BOOLEAN: writer.write(static_cast<uint8_t>(e.boolean ? 1 : 0)); break;
            default:
                throw std::string("Invalid property");
            }
        }
    }
}

// primitive values
FBXProperty::FBXProperty(int16_t a) { type = Type::SHORT; value.i16 = a; }
FBXProperty::FBXProperty(bool a) { type = Type::BOOLEAN; value.boolean = a; }
FBXProperty::FBXProperty(int32_t a) { type = Type::INTEGER; value.i32 = a; }
FBXProperty::FBXProperty(uint32_t a) { type = Type::LONG; value.i64 = a; }
FBXProperty::FBXProperty(float a) { type = Type::FLOAT; value.f32 = a; }
FBXProperty::FBXProperty(double a) { type = Type::DOUBLE; value.f64 = a; }
FBXProperty::FBXProperty(int64_t a) { type = Type::LONG; value.i64 = a; }
// arrays
FBXProperty::FBXProperty(const std::vector<bool> &a) 
    : type(Type::ARRAY_BOOLEAN)
    , values(a.size()) 
{
    for (size_t i = 0; i < a.size(); ++i)
    {
        values[i].boolean = a[i];
    }
}
FBXProperty::FBXProperty(const std::vector<int32_t> &a) 
    : type(Type::ARRAY_INT)
    , values(a.size()) 
{
    for (size_t i = 0; i < a.size(); ++i)
    {
        values[i].i32 = a[i];
    }
}
FBXProperty::FBXProperty(const std::vector<float> &a) 
    : type(Type::ARRAY_FLOAT)
    , values(a.size()) 
{
    for (size_t i = 0; i < a.size(); ++i)
    {
        values[i].f32 = a[i];
    }
}
FBXProperty::FBXProperty(const std::vector<double> &a) 
    : type(Type::ARRAY_DOUBLE)
    , values(a.size()) 
{
    for (size_t i = 0; i < a.size(); ++i)
    {
        values[i].f64 = a[i];
    }
}
FBXProperty::FBXProperty(const std::vector<int64_t> &a) 
	: type(Type::ARRAY_LONG)
    , values(a.size()) 
{
    for (size_t i = 0; i < a.size(); ++i) 
    {
        values[i].i64 = a[i];
    }
}

// raw / string
FBXProperty::FBXProperty(const std::vector<uint8_t> &a, uint8_t type)
	: raw(a) 
{
    if(type != 'R' && type != 'S') {
        throw std::string("Bad argument to FBXProperty constructor");
    }
    this->type = static_cast<Type>(type);
}
// string
FBXProperty::FBXProperty(const std::string &a){
    for(uint8_t v : a) {
        raw.push_back(v);
    }
    this->type = Type::STRING;
}
FBXProperty::FBXProperty(const char *a){
    for(;*a != 0; a++) {
        raw.push_back(*a);
    }
    this->type = Type::STRING;
}

namespace {
    char base16Letter(uint8_t n) {
        n %= 16;
        if(n <= 9) return n + '0';
        return n + 'a' - 10;
    }
    std::string base16Number(uint8_t n) {
        return std::string() + base16Letter(n >> 4) + base16Letter(n);
    }
}

FBXProperty::Type FBXProperty::GetType() const
{
    return type;
}

void FBXProperty::GetData(void* buffer) const
{
    switch (type)
    {
    case Type::SHORT: memcpy(buffer, &value.i16, sizeof(value.i16)); break;
    case Type::BOOLEAN: memcpy(buffer, &value.boolean, sizeof(bool)); break;
    case Type::INTEGER: memcpy(buffer, &value.i32, sizeof(value.i32)); break;
    case Type::FLOAT: memcpy(buffer, &value.f32, sizeof(value.f32)); break;
    case Type::DOUBLE: memcpy(buffer, &value.f64, sizeof(value.f64)); break;
    case Type::LONG: memcpy(buffer, &value.i64, sizeof(value.i64)); break;

    case Type::ARRAY_BOOLEAN:
        {
            bool* ptr = reinterpret_cast<bool*>(buffer);
            for (auto iter = begin(values); iter != end(values); ++iter)
            {
                *ptr = iter->boolean;
                ptr += 1;
            }
        } break;
    case Type::ARRAY_DOUBLE:
        {
            double* ptr = reinterpret_cast<double*>(buffer);
            for (auto iter = begin(values); iter != end(values); ++iter)
            {
                *ptr = iter->f64;
                ptr += 1;
            }
        } break;
    case Type::ARRAY_FLOAT:
        {
            float* ptr = reinterpret_cast<float*>(buffer);
            for (auto iter = begin(values); iter != end(values); ++iter)
            {
                *ptr = iter->f32;
                ptr += 1;
            }
        } break;
    case Type::ARRAY_INT:
        {
            int32_t* ptr = reinterpret_cast<int32_t*>(buffer);
            for (auto iter = begin(values); iter != end(values); ++iter)
            {
                *ptr = iter->i32;
                ptr += 1;
            }
        } break;
    case Type::ARRAY_LONG:
        {
            int64_t* ptr = reinterpret_cast<int64_t*>(buffer);
            for (auto iter = begin(values); iter != end(values); ++iter)
            {
                *ptr = iter->i64;
                ptr += 1;
            }
        } break;
    }
}

string FBXProperty::to_string(bool skip_quotes, bool exit_on_zero_char) const
{
    switch (type)
    {
    case Type::SHORT: return std::to_string(value.i16);
    case Type::BOOLEAN: return value.boolean ? "true" : "false";
    case Type::INTEGER: return std::to_string(value.i32);
    case Type::FLOAT: return std::to_string(value.f32);
    case Type::DOUBLE: return std::to_string(value.f64);
    case Type::LONG: return std::to_string(value.i64);
    case Type::STRING2:
    {
        string s("\"");
        for (char c : raw) {
            s += std::to_string(c) + " ";
        }
        return s + "\"";
    }
    case Type::STRING:
    {
        string s((skip_quotes) ? "" : "\"");
        for (uint8_t c : raw) {
            if (c == '\0' && exit_on_zero_char)
                break;

            if (c == '\\') s += "\\\\";
            else if (c >= 32 && c <= 126) s += c;
            else s = s + "\\u00" + base16Number(c);
        }
        return (skip_quotes) ? s : s + "\"";
    }
    default:
        {
            string s("[");
            bool hasPrev = false;
            for (auto e : values) 
            {
                if (hasPrev) s += ", ";

                switch (type)
                {
                case Type::ARRAY_FLOAT: s += std::to_string(e.f32); break;
                case Type::ARRAY_DOUBLE: s += std::to_string(e.f64); break;
                case Type::ARRAY_LONG: s += std::to_string(e.i64); break;
                case Type::ARRAY_INT: s += std::to_string(e.i32); break;
                case Type::ARRAY_BOOLEAN: s += (e.boolean ? "true" : "false"); break;
                }

                hasPrev = true;
            }
            return s + "]";
        }
    }

    throw std::string("Invalid property");
}

string FBXProperty::to_ascii(uint32_t tab_offset) const
{
	if (type == 'Y') return std::to_string(value.i16);
	else if (type == 'C') return value.boolean ? "1" : "0";
	else if (type == 'I') return std::to_string(value.i32);
	else if (type == 'F') return std::to_string(value.f32);
	else if (type == 'D') return std::to_string(value.f64);
	else if (type == 'L') return std::to_string(value.i64);
	else if (type == 'R') {
		string s("\"");
		for (char c : raw) {
			s += std::to_string(c) + " ";
		}
		return s + "\"";
	}
	else if (type == 'S') {
		string s("\"");
		for (uint8_t c : raw) {
			if (c == '\\') s += "\\\\";
			else if (c >= 32 && c <= 126) s += c;
			else s = s + "\\u00" + base16Number(c);
		}
		return s + "\"";
	}
	else {
		string s("*");
		s += std::to_string(values.size());
		s += " {\n";
		for (uint32_t i = 0; i < tab_offset; ++i)
			s += '\t';
		s += "\ta: ";
		bool hasPrev = false;
		for (auto e : values) {
			if (hasPrev) s += ",";
			if (type == 'f') s += std::to_string(e.f32);
			else if (type == 'd') s += std::to_string(e.f64);
			else if (type == 'l') s += std::to_string(e.i64);
			else if (type == 'i') s += std::to_string(e.i32);
			else if (type == 'b') s += (e.boolean ? "1" : "0");
			hasPrev = true;
		}
		s += '\n';
		for (uint32_t i = 0; i < tab_offset; ++i)
			s += '\t';
		s += '}';
		return s;
	}
	throw std::string("Invalid property");
}

uint32_t FBXProperty::GetBytesCount() const
{
    switch (type)
    {
    case Type::BOOLEAN: return 1 + 1;
    case Type::SHORT: return 2 + 1; // 2 for int16, 1 for type spec
    
    case Type::INTEGER:
    case Type::FLOAT: return 4 + 1;

    case Type::DOUBLE: 
    case Type::LONG: return 8 + 1;

    case Type::STRING2: 
    case Type::STRING: return raw.size() + 5;

    case Type::ARRAY_BOOLEAN: return values.size() * 1 + 13;

    case Type::ARRAY_FLOAT: 
    case Type::ARRAY_INT: return values.size() * 4 + 13;

    case Type::ARRAY_DOUBLE:
    case Type::ARRAY_LONG: return values.size() * 8 + 13;
    default:
        throw std::string("Invalid property");
    }
}

size_t FBXProperty::GetCount() const
{
    switch (type)
    {
    case Type::BOOLEAN:
    case Type::SHORT:
    case Type::INTEGER:
    case Type::FLOAT:
    case Type::DOUBLE:
    case Type::LONG: return 1;

    case Type::STRING2:
    case Type::STRING: return raw.size();

    case Type::ARRAY_BOOLEAN:
    case Type::ARRAY_FLOAT:
    case Type::ARRAY_INT:
    case Type::ARRAY_DOUBLE:
    case Type::ARRAY_LONG: return values.size();
    default:
        throw std::string("Invalid property");
    }
}

// primitive values
void FBXProperty::Set(int16_t a) { type = Type::SHORT; value.i16 = a; }
void FBXProperty::Set(bool b) { type = Type::BOOLEAN; value.boolean = b; }
void FBXProperty::Set(int32_t i) { type = Type::INTEGER; value.i32 = i; }
void FBXProperty::Set(uint32_t i) { type = Type::INTEGER; value.i32 = static_cast<int>(i); }
void FBXProperty::Set(float f) { type = Type::FLOAT; value.f32 = f; }
void FBXProperty::Set(double d) { type = Type::DOUBLE; value.f64 = d; }
void FBXProperty::Set(int64_t i) { type = Type::LONG; value.i64 = i; }
// arrays
void FBXProperty::Set(const std::vector<bool>& arr) 
{ 
    type = Type::ARRAY_BOOLEAN; 
    values.resize(arr.size()); 
    for (size_t i = 0; i < arr.size(); ++i)
        values[i].boolean = arr[i];
}
void FBXProperty::Set(const std::vector<int32_t> &arr)
{
    type = Type::ARRAY_INT;
    values.resize(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
        values[i].i32 = arr[i];
}
void FBXProperty::Set(const std::vector<float> &arr)
{
    type = Type::ARRAY_FLOAT;
    values.resize(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
        values[i].f32 = arr[i];
}
void FBXProperty::Set(const std::vector<double> &arr)
{
    type = Type::ARRAY_DOUBLE;
    values.resize(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
        values[i].f64 = arr[i];
}
void FBXProperty::Set(const std::vector<int64_t> &arr)
{
    type = Type::ARRAY_LONG;
    values.resize(arr.size());
    for (size_t i = 0; i < arr.size(); ++i)
        values[i].i64 = arr[i];
}
// raw / string
void FBXProperty::Set(const std::vector<uint8_t>& arr, uint8_t type)
{
    raw = arr;
    if (type != 'R' && type != 'S') {
        throw std::string("Bad argument to FBXProperty constructor");
    }
    this->type = static_cast<Type>(type);
}
void FBXProperty::Set(const std::string& text)
{
    for (uint8_t v : text) {
        raw.push_back(v);
    }
    this->type = Type::STRING;
}
void FBXProperty::Set(const char* text)
{
    for (; *text != 0; text++) {
        raw.push_back(*text);
    }
    this->type = Type::STRING;
}

} // namespace fbx
