#include <string>

#include "../globals.hpp"

tsmap
divide_timestamp (const std::string source);

long
timestamp_to_ms (const std::string source);

std::string
ms_to_timestamp (const long source, bool no_filling = false);


bool
is_it_a_timestamp (const std::string source);