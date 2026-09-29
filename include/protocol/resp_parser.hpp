#pragma once

#include "resp_types.hpp"

parseResult integerParser(const char* bytes, int length);
parseResult simpleStringParser(const char* bytes, int length);
parseResult bulkStringParser(const char* bytes, int length);
parseResult errorParser(const char* bytes, int length);
parseResult arrayParser(const char* bytes, int length);