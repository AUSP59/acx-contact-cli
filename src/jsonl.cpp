#include "jsonl.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cctype>

namespace acx {

static void skip_ws(const std::string& s, size_t& i){ while(i<s.size() && isspace((unsigned char)s[i])) ++i; }

std::string J::escape(const std::string& s) {
    std::ostringstream o;
    for(char c: s){
        switch(c){
            case '"': o << "\\""; break;
            case '\\': o << "\\\\"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ((unsigned char)c < 0x20){
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else o << c;
        }
    }
    return o.str();
}

static std::string dump_impl(const J& j, int indent, int level){
    std::ostringstream o;
    if (std::holds_alternative<std::nullptr_t>(j.v)) o << "null";
    else if (std::holds_alternative<bool>(j.v)) o << (std::get<bool>(j.v) ? "true":"false");
    else if (std::holds_alternative<double>(j.v)) o << std::get<double>(j.v);
    else if (std::holds_alternative<std::string>(j.v)) o << '"' << J::escape(std::get<std::string>(j.v)) << '"';
    else if (std::holds_alternative<std::vector<J>>(j.v)){
        const auto& a = std::get<std::vector<J>>(j.v);
        o << "[";
        if (!a.empty() && indent) o << "\n";
        for(size_t i=0;i<a.size();++i){
            if (indent) o << std::string((level+1)*indent, ' ');
            o << dump_impl(a[i], indent, level+1);
            if (i+1<a.size()) o << ",";
            if (indent) o << "\n";
        }
        if (!a.empty() && indent) o << std::string(level*indent, ' ');
        o << "]";
    } else {
        const auto& m = std::get<std::map<std::string, J>>(j.v);
        o << "{";
        if (!m.empty() && indent) o << "\n";
        size_t i=0;
        for (auto& kv : m){
            if (indent) o << std::string((level+1)*indent, ' ');
            o << '"' << J::escape(kv.first) << '"' << ":" << (indent ? " " : "");
            o << dump_impl(kv.second, indent, level+1);
            if (++i<m.size()) o << ",";
            if (indent) o << "\n";
        }
        if (!m.empty() && indent) o << std::string(level*indent, ' ');
        o << "}";
    }
    return o.str();
}

std::string J::dump(int indent) const { return dump_impl(*this, indent, 0); }

static J parse_value(const std::string& s, size_t& i);

static std::string parse_string(const std::string& s, size_t& i){
    if (s[i] != '"') throw std::runtime_error("Expected string");
    ++i; std::ostringstream o;
    while(i<s.size()){
        char c = s[i++];
        if (c == '"') break;
        if (c == '\\'){
            if (i>=s.size()) break;
            char e=s[i++];
            switch(e){
                case '"': o<<'"'; break;
                case '\\': o<<'\\'; break;
                case '/': o<<'/'; break;
                case 'b': o<<'\b'; break;
                case 'f': o<<'\f'; break;
                case 'n': o<<'\n'; break;
                case 'r': o<<'\r'; break;
                case 't': o<<'\t'; break;
                case 'u': { // skip unicode, copy raw
                    for(int k=0;k<4 && i<s.size();++k) ++i;
                } break;
                default: o<<e; break;
            }
        } else {
            o<<c;
        }
    }
    return o.str();
}

static double parse_number(const std::string& s, size_t& i){
    size_t j=i;
    while(j<s.size() && (isdigit((unsigned char)s[j]) || s[j]=='-'||s[j]=='+'||s[j]=='.'||s[j]=='e'||s[j]=='E')) ++j;
    double d = std::stod(s.substr(i,j-i));
    i = j; return d;
}

static J parse_array(const std::string& s, size_t& i){
    if (s[i] != '[') throw std::runtime_error("Expected [");
    ++i; std::vector<J> a; skip_ws(s,i);
    if (s[i] == ']'){ ++i; return a; }
    while(true){
        skip_ws(s,i);
        a.push_back(parse_value(s,i));
        skip_ws(s,i);
        if (s[i] == ','){ ++i; continue; }
        if (s[i] == ']'){ ++i; break; }
        throw std::runtime_error("Expected , or ]");
    }
    return J(a);
}

static J parse_object(const std::string& s, size_t& i){
    if (s[i] != '{') throw std::runtime_error("Expected {");
    ++i; std::map<std::string,J> m; skip_ws(s,i);
    if (s[i] == '}'){ ++i; return m; }
    while(true){
        skip_ws(s,i);
        std::string key = parse_string(s,i);
        skip_ws(s,i);
        if (s[i] != ':') throw std::runtime_error("Expected :");
        ++i;
        skip_ws(s,i);
        m[key] = parse_value(s,i);
        skip_ws(s,i);
        if (s[i] == ','){ ++i; continue; }
        if (s[i] == '}'){ ++i; break; }
        throw std::runtime_error("Expected , or }");
    }
    return J(m);
}

static J parse_value(const std::string& s, size_t& i){
    skip_ws(s,i);
    if (i>=s.size()) throw std::runtime_error("Unexpected end");
    char c = s[i];
    if (c=='"') return J(parse_string(s,i));
    if (c=='{' ) return parse_object(s,i);
    if (c=='[' ) return parse_array(s,i);
    if (c=='t' && s.substr(i,4)=="true"){ i+=4; return J(true); }
    if (c=='f' && s.substr(i,5)=="false"){ i+=5; return J(false); }
    if (c=='n' && s.substr(i,4)=="null"){ i+=4; return J(nullptr); }
    if (c=='-'||c=='+'||isdigit((unsigned char)c)) return J(parse_number(s,i));
    throw std::runtime_error("Invalid JSON");
}

J J::parse(const std::string& s, size_t& i){ return parse_value(s,i); }
J J::parse(const std::string& s){ size_t i=0; return parse_value(s,i); }

std::vector<J> parse_jsonl(const std::string& data){
    std::vector<J> out; size_t pos=0;
    while (pos < data.size()){
        size_t end = data.find('\n', pos);
        if (end == std::string::npos) end = data.size();
        std::string line = data.substr(pos, end-pos);
        if (!line.empty()) {
            size_t i=0;
            out.push_back(J::parse(line));
        }
        pos = end+1;
    }
    return out;
}

std::string dump_jsonl(const std::vector<J>& arr){
    std::ostringstream o;
    for (auto& j : arr) o << j.dump(0) << "\n";
    return o.str();
}

} // namespace acx
