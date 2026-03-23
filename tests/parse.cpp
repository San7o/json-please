// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#include <json-please/json-please.hpp>

#include <valfuzz/valfuzz.hpp>

class Person
{
public:
  std::string name;
  int         age;
  float       height;

  Person(const std::string& name, int age, float height)
    : name(name), age(age), height(height) {}
  Person(jp::Json& json)
  {
    this->from_json(json);
  }
    
  jp::Json to_json()
  {
    return jp::Json({{"name",   this->name},
                     {"age",    this->age},
                     {"height", this->height}});
  }
  void from_json(jp::Json& json)
  {
    this->name   = std::get<std::string>(json["name"]);
    this->age    = std::get<int>(json["age"]);
    this->height = std::get<float>(json["height"]);
  }
};

TEST(to_string, "json_to_string")
{
  auto person_a = Person("Foo", 69, 1.94f);
  auto json     = person_a.to_json();
  auto str      = json.serialize();
  ASSERT(str == "{\"age\": 69, \"height\": 1.94, \"name\": \"Foo\"}");
}

TEST(from_string, "json_from_string")
{
  auto json = jp::Json::parse("{\"age\": 69, \"height\": 1.95, \"name\": \"Foo\"}");
  ASSERT(json);
  
  auto person = Person(json.value());
  ASSERT(person.name == "Foo");
  ASSERT(person.age  == 69);
  ASSERT(std::abs(person.height - 1.95) < 0.0001);
}

TEST(arrays, "json_vector_test")
{
  // Testing a list of strings
  std::string raw = "{\"scores\": [10, 20, 30], \"tags\": [\"cpp\", \"json\"]}";
  auto json_opt = jp::Json::parse(raw);
  ASSERT(json_opt.has_value());
  auto& json = json_opt.value();

  auto scores = std::get<jp::Json::Array>(json["scores"]).values;
  ASSERT(scores.size() == 3);
  ASSERT(std::get<int>(scores[1]) == 20);

  auto tags = std::get<jp::Json::Array>(json["tags"]).values;
  ASSERT(tags.size() == 2);
  ASSERT(std::get<std::string>(tags[0]) == "cpp");
}

TEST(edge_cases, "json_empty_and_special")
{
  // Empty Object
  auto empty_json = jp::Json::parse("{}");
  ASSERT(empty_json.has_value());
  ASSERT(empty_json->serialize() == "{}");

  // Strings with spaces
  std::string raw = "{\"message\": \"Hello World\"}";
  auto json = jp::Json::parse(raw).value();
  ASSERT(std::get<std::string>(json["message"]) == "Hello World");
}

TEST(failure, "json_invalid_syntax")
{
  // Missing closing brace
  auto json = jp::Json::parse("{\"name\": \"Foo\""); 
  ASSERT(!json.has_value());

  // Missing colon
  auto json2 = jp::Json::parse("{\"name\" \"Foo\"}");
  ASSERT(!json2.has_value());
}

TEST(nested_objects, "json_nested_and_bool")
{
  std::string raw = "{\"active\": true, \"metadata\": {\"id\": 101, \"tag\": \"beta\"}}";
  auto json_opt = jp::Json::parse(raw);
    
  ASSERT(json_opt.has_value());
  auto& json = json_opt.value();

  ASSERT(std::get<bool>(json["active"]) == true);
    
  // Accessing nested object
  auto metadata = std::get<jp::Json>(json["metadata"]);
  ASSERT(std::get<int>(metadata["id"]) == 101);
  ASSERT(std::get<std::string>(metadata["tag"]) == "beta");
}
