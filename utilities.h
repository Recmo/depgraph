#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <vector>
#include <map>
#include <sstream>
#include <tr1/memory>
#include <cassert>

using namespace std;
using namespace std::tr1;

// const int max_t = 11*12;
const int max_t = 13*12;

#define fast_true(x) __builtin_expect((x),1)
#define fast_false(x) __builtin_expect((x),0)

typedef unsigned long int uint64;
typedef signed long int sint64;

inline uint64 endian_swap(uint64 value)
{
	return __builtin_bswap64(value);
}

inline sint64 endian_swap(sint64 value)
{
	return __builtin_bswap64(value);
}

inline int count_leading_zeros(uint64 value)
{
	// if(fast_false(value == 0)) return 64;
	return __builtin_clzl(value);
}

inline int count_leading_ones(uint64 value)
{
	return count_leading_zeros(~value);
}

template<class T>
inline bool contains(vector<T>& v, const T& value)
{
	for(typename vector<T>::iterator i = v.begin(); i != v.end(); ++i)
	{
		if(*i == value) return true;
	}
	return false;
}

template<class T>
inline ostream& operator<<(ostream& out, vector<T> in)
{
	if(in.size() == 0) return out << "[]";
	out << "[";
	int i;
	for(i=0; i < in.size()-1; i++)
	{
		out << in[i] << ", ";
	}
	out << in[i] << "]";
	return out;
}

template<class T>
inline ostream& operator<<(ostream& out, vector<T*> in)
{
	if(in.size() == 0) return out << "[]";
	out << "[";
	int i;
	for(i=0; i < in.size()-1; i++)
	{
		out << *in[i] << ", ";
	}
	out << *in[i] << "]";
	return out;
}

inline bool ends_with(const string& haystack, const string& needle)
{
	if(needle.length() > haystack.length()) return false;
	return haystack.substr(haystack.length() - needle.length(), needle.length()) == needle;
}


