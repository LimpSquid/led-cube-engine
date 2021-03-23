#include <http/uri.h>
#include <boost/regex.hpp>
#include <utility>


using namespace rest::http;
using namespace boost;

uri::uri(const std::string &raw) :
    raw_(raw)
{
    parse_raw();
}

void uri::parse_raw()
{
    static const char *RFC3986 = R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"; // See https://tools.ietf.org/html/rfc3986#page-50

    scheme_.clear();
    authority_.clear();
    path_.clear();
    query_.clear();
    fragment_.clear();

    if(raw_.empty())
        return;

    const regex uri_regex(RFC3986, regex::extended);
    smatch segments;

    if(!regex_match(raw_, segments, uri_regex))
        return;

    for(int i = 0; i < segments.size(); ++i) {
        const std::string &segment = segments[i];

        switch(i) {
            default:                            break;
            case 2:     scheme_ = segment;      break;
            case 4:     authority_ = segment;   break;
            case 5:     path_ = segment;        break;
            case 7:     query_ = segment;       break;
            case 9:     fragment_ = segment;    break;
        }
    }
}