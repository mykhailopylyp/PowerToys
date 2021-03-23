#include "pch.h"

void traceFunction(std::string file, long line)
{
    Logger::warn("TRACE {}:{} {}", file, line, GetLastError());
}