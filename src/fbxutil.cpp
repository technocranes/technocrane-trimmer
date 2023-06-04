#include "fbxutil.h"


namespace fbx {

namespace {
    bool isLittleEndian()
    {
        uint16_t number = 0x1;
        char *numPtr = (char*)&number;
        return (numPtr[0] == 1);
    }
}

uint8_t Reader::readUint8()
{
    return getc();
}

int8_t Reader::readInt8()
{
    return getc();
}

uint16_t Reader::readUint16()
{
    uint16_t i;
    char *c = (char *)(&i);
    if(isLittleEndian()) {
        read(c, 2);
    } else {
        c[1] = getc();
        c[0] = getc();
    }
    return i;
}

int16_t Reader::readInt16()
{
    int16_t i;
    char *c = (char *)(&i);
    if(isLittleEndian()) {
        read(c, 2);
    } else {
        c[1] = getc();
        c[0] = getc();
    }
    return i;
}

uint32_t Reader::readUint32()
{
    uint32_t i;
    char *c = (char *)(&i);
    if(isLittleEndian()) {
        read(c, 4);
    } else {
        c[3] = getc();
        c[2] = getc();
        c[1] = getc();
        c[0] = getc();
    }
    return i;
}

int32_t Reader::readInt32()
{
    int32_t i;
    char *c = (char *)(&i);
    if(isLittleEndian()) {
        read(c, 4);
    } else {
        c[3] = getc();
        c[2] = getc();
        c[1] = getc();
        c[0] = getc();
    }
    return i;
}

uint64_t Reader::readUint64()
{
    uint64_t i;
    char *c = (char *)(&i);
    if(isLittleEndian()) {
        read(c, 8);
    } else {
        c[7] = getc();
        c[6] = getc();
        c[5] = getc();
        c[4] = getc();
        c[3] = getc();
        c[2] = getc();
        c[1] = getc();
        c[0] = getc();
    }
    return i;
}


std::string Reader::readString(uint32_t length)
{
    char *buffer = new char[length + 1];
    buffer[length] = 0;
    if(length) read(buffer, length);

	std::string str(buffer);
	if (buffer)
	{
		delete[] buffer;
		buffer = nullptr;
	}
    return str;
}

float Reader::readFloat()
{
    float f;
    char *c = (char *)(&f);
    if(isLittleEndian()) {
        read(c, 4);
    } else {
        c[3] = getc();
        c[2] = getc();
        c[1] = getc();
        c[0] = getc();
    }
    return f;
}

double Reader::readDouble()
{
    double f;
    char *c = (char *)(&f);
    if(isLittleEndian()) {
        read(c, 8);
    } else {
        c[7] = getc();
        c[6] = getc();
        c[5] = getc();
        c[4] = getc();
        c[3] = getc();
        c[2] = getc();
        c[1] = getc();
        c[0] = getc();
    }
    return f;
}

Reader::Reader(std::ifstream *input)
    :ifstream(input),buffer(NULL),i(0)
{}

Reader::Reader(char *input)
    :ifstream(NULL),buffer(input),i(0)
{}

uint8_t Reader::getc()
{
    uint8_t tmp;
    if(ifstream != NULL) (*ifstream) >> tmp;
    else tmp = buffer[i++];
    return tmp;
}

void Reader::read(char *s, uint32_t n)
{
    if(ifstream != NULL) {
        ifstream->read(s, n);
    } else for(uint32_t a = 0; a < n; a++) {
        s[a] = buffer[i++];
    }
}

Writer::Writer(std::ostream *output)
	:stream(output)
{}

void Writer::putc(uint8_t c)
{
    (*stream) << c;
}

void Writer::write(std::uint8_t a)
{
    putc(a);
}

void Writer::write(std::int8_t a)
{
    putc(a);
}

void Writer::write(std::uint16_t a)
{
    putc(static_cast<uint8_t>(a));
    putc(a >> 8);
}

void Writer::write(std::int16_t _a)
{
    uint16_t a = *(int16_t*)((char*) &_a);
    putc(static_cast<uint8_t>(a));
    putc(a >> 8);
}

void Writer::write(std::uint32_t a)
{
    putc(a);
    putc(a >> 8);
    putc(a >> 16);
    putc(a >> 24);
}

void Writer::write(std::int32_t _a)
{
    uint32_t a = *(uint32_t*)((char*) &_a);
    putc(a);
    putc(a >> 8);
    putc(a >> 16);
    putc(a >> 24);
}

void Writer::write(std::uint64_t a)
{
    putc(static_cast<uint8_t>(a));
    putc(static_cast<uint8_t>(a >> 8));
    putc(static_cast<uint8_t>(a >> 16));
    putc(static_cast<uint8_t>(a >> 24));
    putc(static_cast<uint8_t>(a >> 32));
    putc(static_cast<uint8_t>(a >> 40));
    putc(static_cast<uint8_t>(a >> 48));
    putc(static_cast<uint8_t>(a >> 56));
}

void Writer::write(std::int64_t _a)
{
    uint64_t a = *(uint64_t*)((char*) &_a);
    putc(static_cast<uint8_t>(a));
    putc(static_cast<uint8_t>(a >> 8));
    putc(static_cast<uint8_t>(a >> 16));
    putc(static_cast<uint8_t>(a >> 24));
    putc(static_cast<uint8_t>(a >> 32));
    putc(static_cast<uint8_t>(a >> 40));
    putc(static_cast<uint8_t>(a >> 48));
    putc(static_cast<uint8_t>(a >> 56));
}

void Writer::write(const std::string &a)
{
    for(char c : a) {
        putc(c);
    }
}

void Writer::write(float a)
{
    char *c = (char *)(&a);
    if(isLittleEndian()) {
        for(int i = 0; i < 4; i++) {
            putc(c[i]);
        }
    } else {
        for(int i = 3; i >= 0; i--) {
            putc(c[i]);
        }
    }
}

void Writer::write(double a)
{
    char *c = (char *)(&a);
    if(isLittleEndian()) {
        for(int i = 0; i < 8; i++) {
            putc(c[i]);
        }
    } else {
        for(int i = 7; i >= 0; i--) {
            putc(c[i]);
        }
    }
}

void Writer::writeBlockSentinelData(const int blockLength)
{
	for (int i = 0; i < blockLength; ++i)
		putc('\0');
}

double fbxTimeToSeconds(i64 value)
{
    return double(value) / 46186158000L;
}


i64 secondsToFbxTime(double value)
{
    return i64(value * 46186158000L);
}

} // namespace fbx
