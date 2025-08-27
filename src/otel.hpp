#pragma once
#include <string>
#include <map>

namespace acx {
bool otel_enabled();
void otel_event(const std::string& name, const std::map<std::string, std::string>& attrs);
void otel_metric(const std::string& name, double value, const std::map<std::string, std::string>& attrs);
}
