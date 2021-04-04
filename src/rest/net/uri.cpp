#include <net/uri.h>
#include <regex>
#include <utility>

using namespace rest::net;

uri::uri(const std::string &raw) :
    raw_(raw),
    valid_(false)
{
    parse_raw();
}

bool uri::valid() const
{
    return valid_;
}
const std::string &uri::raw() const
{
    return raw_;
}

const std::string &uri::scheme() const
{
    return scheme_;
}

const std::string &uri::authority() const
{
    return authority_;
}

const std::string &uri::path() const
{
    return path_;
}

const std::string &uri::query() const
{
    return query_;
}

const std::string &uri::fragment() const
{
    return fragment_;
}

void uri::parse_raw()
{
    static const char *RFC3986 = R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"; // See https://tools.ietf.org/html/rfc3986#page-50

    scheme_.clear();
    authority_.clear();
    path_.clear();
    query_.clear();
    fragment_.clear();
    valid_ = false;

    if(raw_.empty())
        return;

    const std::regex uri_regex(RFC3986, std::regex::extended);
    std::smatch segments;

    if(!std::regex_match(raw_, segments, uri_regex))
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

    valid_ = true;
}