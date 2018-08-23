

#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <iostream>

const char *readInt(const char *buf, uint32_t *val);
const char *read64Int(const char *buf, uint64_t *val);
const char *readBytes(const char *buf, char *out, uint32_t len);
const char *readCString(const char *buf, char **out);
const char *readString(const char *buf, std::string& data);

char *writeInt(char *buf, uint32_t value);
char *write64Int(char *buf, uint64_t value);
char *writeBytes(char *buf, const char *in, uint32_t len);
char *writeCString(char *buf, const char *in, uint32_t len);
char *writeString(char *buf, const std::string& data);

#ifndef __APPLE__
uint64_t ntohll(const uint64_t input);
uint64_t htonll(const uint64_t input);
#endif

uint64_t time_now();

#endif


