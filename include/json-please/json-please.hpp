// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#pragma once

#include <string>
#include <variant>
#include <map>
#include <sstream>
#include <vector>

namespace jp
{

class Json
{
public:

  struct Array;
  using Key   = std::string;
  using Value = std::variant<int,
                             float,
                             Json,
                             bool,
                             std::string,
                             Array>;

  struct Array {
    std::vector<Value> values;
  };

  Json() = default;
  Json(std::map<Key, Value> items);
  
  Value& operator[](const std::string& key);

  std::string serialize();
  static std::optional<Json> parse(const std::string& json_str);

  friend std::ostream& operator<<(std::ostream& os, const Json& json);
  
private:

  std::map<Key, Value> items;
  
};
  
} // namespace jp
