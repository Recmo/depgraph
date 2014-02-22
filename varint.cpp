#include "utilities.h"
#include "varint.h"

#include <string.h>
#include <cstdlib>
#include <iostream>
using namespace std;

/*

<tag> {data}

where data can be 

uint: <value>
sint: <value>
fixed: [bytes]
binary: <length> [bytes]
tuple: {data} {data} {data} ...
list: <length> {data} {data} ...

children: binary consisting of tag-data pairs
string: binary consisting of uints
datetime: sint of seconds since 2000
nanotime: sint of nanoseconds since 2000

0x1xxx xxxx
0x01xx xxxx <1 byte>
0x001x xxxx <2 bytes>
...
0x0000 0001 <7 bytes>
0x0000 0000 <8 bytes>


TODO: Fast write with offset:

 0000 00xx xxxx xxxx
-0100 0000 1000 0000
=1100 00cc cccc cccc

should become:
 0100 00cc cccc cccc

 ^= -2 << (num_bytes * 7)

*/

// This constant will add the proper offsets
// for the different cases and move the
// leading one one position to the right
const uint64 offset = 0x0102040810204080;

int length_uint(uint64 value)
{
	// return num_bytes[count_leading_zeros(~(value - offset) ^ offset)];
	value -= offset;
	int num_bytes = 9 - count_leading_zeros(~value ^ offset) / 7;
	if(fast_true(num_bytes > 0)) 
	{
		return num_bytes;
	}
	return 9;
}

uint64 read_uint(char* &buffer)
{
	uint64 value = *reinterpret_cast<uint64*>(buffer);
	value = endian_swap(value);
	int leading_zeros = count_leading_zeros(value);
	buffer += leading_zeros + 1;
	if(fast_true(leading_zeros  < 8))
	{
		value <<= leading_zeros;
		value += offset << (56 - leading_zeros * 7);
		value >>= 56 - leading_zeros * 7;
		return value;
	}
	value = *reinterpret_cast<uint64*>(buffer - 8);
	value = endian_swap(value);
	value += offset;
	if(fast_false(value < offset))
	{
		throw "Overflow";
	}
	return value;
}

void write_uint(char* &buffer, uint64 value)
{
	value -= offset;
	int num_bytes = 9 - count_leading_zeros(~value ^ offset) / 7;
	if(fast_true(num_bytes))
	{
		value ^= ((uint64)-2) << num_bytes * 7;
		value <<= (8 - num_bytes) * 8;
		value = endian_swap(value);
		*reinterpret_cast<uint64*>(buffer) = value;
		buffer += num_bytes;
		return;
	}
	*buffer = 0x00;
	value = endian_swap(value);
	*reinterpret_cast<uint64*>(buffer + 1) = value;
	buffer += 9;
}

int length_sint(sint64 value)
{
	sint64 v = abs(value) + (value >> 63);
	int num_bytes = (64 - count_leading_zeros(v)) / 7 + 1;
	return (num_bytes > 9) ? 9 : num_bytes;
}

// Only the type is changed, this has the effect of making
// the right shift an arithmetic shift
sint64 read_sint(char* &buffer)
{
	sint64 value = *reinterpret_cast<sint64*>(buffer);
	value = endian_swap(value);
	int num_bytes = count_leading_zeros(value) + 1;
	buffer += num_bytes;
	if(fast_true(num_bytes < 9))
	{
		value <<= num_bytes;
		value >>= 64 - num_bytes*7;
		return value;
	}
	value = *reinterpret_cast<sint64*>(buffer - 8);
	value = endian_swap(value);
	return value;
}

void write_sint(char* &buffer, sint64 value)
{
	sint64 v = abs(value) + (value >> 63);
	int num_bytes = (64 - count_leading_zeros(v)) / 7 + 1;
	if(fast_true(num_bytes < 9))
	{
		value &= (((uint64)1) << (num_bytes*7)) - 1;
		value |= ((uint64)1) << (num_bytes*7);
		value <<= (8 - num_bytes) * 8;
		value = endian_swap(value);
		*reinterpret_cast<uint64*>(buffer) = value;
		buffer += num_bytes;
		return;
	}
	*buffer = 0xFF;
	value = endian_swap(value);
	*reinterpret_cast<uint64*>(buffer + 1) = value;
	buffer += num_bytes;
}

string read_string(char* &cursor)
{
	uint64 length = read_uint(cursor);
	char* start = cursor;
	cursor += length;
	return string(start, length);
}

void read_string(char* &cursor, const string& s)
{
	write_uint(cursor, s.length());
	memcpy(cursor, s.data(), s.length());
	cursor += s.length();
}
