#pragma once
#include <string>
#include <vector>
#include "ByteBuffer.h"

using namespace std;

void split(string& str, string& delim, vector<string>* ret);
string doCommFunc(string funcName, string arg_str);
void dispatchCommFuncEvent(string event_name, string result_str);
string doCommFuncByEvent(string funcName, string arg_str, string event_name);

int lshift(int value, int n);
int rshift(int value, int n);

int compressStr(string& src, ByteBuffer* dest, int& len_compress);
int uncompressStr(ByteBuffer* src, ByteBuffer* dest, int len_compress, int len_src);

int b64CompressStr(const string& src, string& dest);
int unb64UncompressStr(const string& src, const unsigned long srcLen, string& dest);