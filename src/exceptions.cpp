#include "mge/exceptions.hpp"

#include <format>

mge::NotImplementedException::NotImplementedException() : std::logic_error("Function not implemented") {}

mge::NotImplementedException::NotImplementedException(const char *function) : std::logic_error(std::format("{0} not implemented", function)) {}
