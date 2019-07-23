#include "http/client_http.hpp"
#include "http/server_http.hpp"
#include <mutex>

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>
#include <vector>
#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

#include "HandSimulation.h"
