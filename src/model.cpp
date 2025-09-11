#include "model.hpp"
#include "utils.hpp"
#include <map>

namespace acx {

J HistoryEntry::to_json() const {
    return std::map<std::string, J>{
        {"timestamp", timestamp},
        {"user", user},
        {"action", action}
    };
}

HistoryEntry HistoryEntry::from_json(const J& j) {
    const auto& o = std::get<std::map<std::string, J>>(j.v);
    HistoryEntry h;
    h.timestamp = std::get<std::string>(o.at("timestamp").v);
    h.user      = std::get<std::string>(o.at("user").v);
    h.action    = std::get<std::string>(o.at("action").v);
    return h;
}

J Contact::to_json() const {
    std::vector<J> hist;
    for (auto& h : history) hist.push_back(h.to_json());
    return std::map<std::string, J>{
        {"id", id},
        {"name", name},
        {"email", email},
        {"phone", phone},
        {"note", note},
        {"created_at", created_at},
        {"history", hist}
    };
}

Contact Contact::from_json(const J& j){
    const auto& o = std::get<std::map<std::string, J>>(j.v);
    Contact c;
    c.id         = std::get<std::string>(o.at("id").v);
    c.name       = std::get<std::string>(o.at("name").v);
    c.email      = std::get<std::string>(o.at("email").v);
    c.phone      = std::get<std::string>(o.at("phone").v);
    c.note       = std::get<std::string>(o.at("note").v);
    c.created_at = std::get<std::string>(o.at("created_at").v);
    for (auto& jh : std::get<std::vector<J>>(o.at("history").v)) {
        c.history.push_back(HistoryEntry::from_json(jh));
    }
    return c;
}

} // namespace acx
