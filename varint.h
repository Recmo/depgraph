#include "utilities.h"
#pragma once

int length_uint(const uint64 value);

uint64 read_uint(char* &buffer);

// Writes value varint encoded to buffer and advances the buffer
// Note: zeros up to six aditional bytes in the buffer
void write_uint(char* &buffer, uint64 value);

int length_sint(const sint64 value);

sint64 read_sint(char* &buffer);

void write_sint(char* &buffer, sint64 signed_value);

string read_string(char* &cursor);

void write_string(char* &cursor, const string& str);

