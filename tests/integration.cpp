// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#include <json-please/json-please.hpp>

#include <valfuzz/valfuzz.hpp>

TEST(serialization, "Simple serialization test")
{
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

  auto person_a = Person("Foo", 69, 1.94f);
  auto json     = person_a.to_json();
  auto person_b = Person(json);

  ASSERT(person_a.name   == person_b.name);
  ASSERT(person_a.age    == person_b.age);
  ASSERT(person_a.height == person_b.height);
}
