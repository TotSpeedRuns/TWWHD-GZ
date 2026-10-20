#pragma once
#include <stdexcept>
#include <string>
struct ErrorLog {static ErrorLog& getInstance(){static ErrorLog instance;return instance;}void log(const std::string& value){throw std::runtime_error(value);}};
#define LOG_ERR_AND_RETURN(value) return value
