// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#include <json-please/json-please.hpp>

using namespace jp;

Json::Json(std::map<Key, Value> items)
{
  this->items = items;
}

Json::Value& Json::operator[](const std::string& key)
{
  return this->items[key];
}

std::string Json::serialize()
{
  std::ostringstream os;
  os << *this;
  return os.str();
}

namespace jp
{

std::ostream& operator<<(std::ostream& os, const Json::Value& value)
{
  if (auto* v = std::get_if<int>(&value))
  {
    os << *v;
  } else if (auto* v = std::get_if<std::string>(&value))
  {
    os << "\"" << *v << "\"";
  } else if (auto* v = std::get_if<float>(&value))
  {
    os << *v;
  } else if (auto* v = std::get_if<bool>(&value))
  {
    os << ((*v) ? "true" : "false");
  } else if (auto* v = std::get_if<jp::Json>(&value))
  {
    os << *v;
  }
  
  return os;
}
  
std::ostream& operator<<(std::ostream& os, const Json& json)
{
  bool first = true;
  os << "{";
  for (const auto& item : json.items)
  {
    if (!first)
      os << ", ";

    if (auto* v = std::get_if<Json::Array>(&item.second))
    {
      bool first_i = true;
      os << "[";
      for (const auto& i : v->values)
      {
        if (!first_i)
          os << ",";
        
        os << "\"" << item.first << "\": " << i;
        
        first_i = false;
      }
      os << "]";
    }
    else
    {
      os << "\"" << item.first << "\": " << item.second;
    }

    first = false;
  }
  os << "}";

  return os;
}

} // namespace jp

//
// Json deserialization
// --------------------
//
// Json grammar looks like this:
//
//   OPEN_PAREN   ::= {
//   CLOSE_PAREN  ::= }
//   BEGIN_ARRAY  ::= [
//   END_ARRAY    ::= ]
//   COLON        ::= :
//   COMMA        ::= ,
//   EMPTY        ::= null
//   FLOATING     ::= INTEGER . INTEGER
//   INTEGER      ::= [+-][1234567890][1234567890]*
//   BOOLEAN      ::= 'true' | 'false'
//   STRING       ::= IDENTIFIER
//   IDENTIFIER   ::= '"' [ASCII]* '"'
//
//   ARRAY        ::= BEGIN_ARRAY (VALUE COMMA)* END_ARRAY
//   VALUE        ::= BOOLEAN
//                  | INTEGER
//                  | FLOATING
//                  | STRING
//                  | ARRAY
//                  | JSON_OBJECT
//                  | EMPTY
//   JSON_ITEM    ::= IDENTIFIER COLON VALUE
//   JSON_OBJECT  ::= OPEN_PAREN (JSON_ITEM COMMA)* CLOSE_PAREN
//   START        ::= JSON_OBJECT
//

enum JsonTokenType
{
  TOKEN_EMPTY = 0,
  TOKEN_OPEN_PAREN,  // {
  TOKEN_CLOSE_PAREN, // }
  TOKEN_BEGIN_ARRAY, // [
  TOKEN_END_ARRAY,   // ]
  TOKEN_STRING,      // "hello"
  TOKEN_INTEGER,     // 123
  TOKEN_FLOATING,    // 123.123
  TOKEN_BOOLEAN,     // true | false
  TOKEN_COMMA,       // ,
  TOKEN_COLON,       // :
};


std::ostream& operator<<(std::ostream& os, JsonTokenType token)
{
  switch(token)
  {
  case TOKEN_EMPTY:       os << "EMPTY";       break;
  case TOKEN_OPEN_PAREN:  os << "OPEN_PAREN";  break;
  case TOKEN_CLOSE_PAREN: os << "CLOSE_PAREN"; break;
  case TOKEN_BEGIN_ARRAY: os << "BEGIN_ARRAY"; break;
  case TOKEN_END_ARRAY:   os << "END_ARRAY";   break;
  case TOKEN_STRING:      os << "STRING";      break;
  case TOKEN_INTEGER:     os << "INTEGER";     break;
  case TOKEN_FLOATING:    os << "FLOATING";    break;
  case TOKEN_BOOLEAN:     os << "BOOLEAN";     break;
  case TOKEN_COMMA:       os << "COMMA";       break;
  case TOKEN_COLON:       os << "COLON";       break;
  }
  return os;
}

struct JsonToken
{
  using Value = std::variant<std::string,
                             bool,
                             int,
                             float>;
  JsonTokenType type;
  Value value;

  JsonToken(JsonTokenType type)
    : type(type) {}
  JsonToken(JsonTokenType type, Value value)
    : type(type), value(value) {}
};

// Add print debugs
// #define JSON_DEBUG_MODE

#include <deque>
#include <ctype.h>
#include <string.h>

#ifdef JSON_DEBUG_MODE
#include <iostream>
#endif

class JsonLexer
{
public:
  
  const std::string& json;
  unsigned int position;

  JsonLexer(const std::string& json)
    : json(json), position(0) {}

  std::optional<JsonToken> next()
  {
    if (this->position >= json.size())
      return {};

    // Remove spaces
    while (this->position < json.size()
           && isspace(this->json[position]))
      this->position++;
    
    if (this->json[position] == '{')
    {
      this->position++;
      return JsonToken(TOKEN_OPEN_PAREN);
    }
    if (this->json[position] == '}')
    {
      this->position++;
      return JsonToken(TOKEN_CLOSE_PAREN);
    }
    if (this->json[position] == '[')
    {
      this->position++;
      return JsonToken(TOKEN_BEGIN_ARRAY);
    }
    if (this->json[position] == ']')
    {
      this->position++;
      return JsonToken(TOKEN_END_ARRAY);
    }
    if (this->json[position] == ',')
    {
      this->position++;
      return JsonToken(TOKEN_COMMA);
    }
    if (this->json[position] == ':')
    {
      this->position++;
      return JsonToken(TOKEN_COLON);
    }

    if (this->json[position] == '\"')
    {
      bool escaped = false;
      std::string str = "";
      this->position += 1; // account for the '"'
      
      while (this->position < this->json.size()
             && this->json[position] != '"' && !escaped)
      {
        if (this->json[position] == '\\' && !escaped)
          escaped = true;
        else
          escaped = false;

        str += this->json[position];
        this->position++;
      }

      this->position += 1; // account for the '"'
      return JsonToken(TOKEN_STRING, str);
    }

    if (isdigit(this->json[position]))
    {
      // Integer part
      int int_val = 0;
      while (this->position < this->json.size()
             && isdigit(this->json[position]))
      {
        int digit = this->json[position];
        int_val *= 10;
        int_val += digit - '0';
        this->position++;
      }

      // Floating
      if (this->position < this->json.size()
          && this->json[position] == '.')
      {
        float float_val = int_val;
        int   divider   = 10;
        this->position++; // account for the '.'
                
        while (this->position < this->json.size()
               && isdigit(this->json[position]))
        {
          int digit = this->json[position] - '0';
          if (digit != 0)
            float_val += digit / (float)divider;
          divider *= 10;
          this->position++;
        }
        
        return JsonToken(TOKEN_FLOATING, float_val);
      }

      return JsonToken(TOKEN_INTEGER, int_val);
    }

    if (this->json.size() - position >= 4)
    {
      const char* cstr = this->json.c_str();
      if (strncmp(cstr + position, "true", 4) == 0)
      {
        this->position += 4;
        return JsonToken(TOKEN_BOOLEAN, true);
      }
      if (strncmp(cstr + position, "null", 4) == 0)
      {
        this->position += 4;
        return JsonToken(TOKEN_EMPTY);
      }
      if (this->json.size() - position >= 5)
      {
        if (strncmp(cstr + position, "false", 5) == 0)
        {
          this->position += 5;
          return JsonToken(TOKEN_BOOLEAN, false);
        }
      }
    }

    return {};
  }
  
};

static std::deque<JsonToken> tokenize(const std::string& json_str)
{
  std::deque<JsonToken> tokens = {};
  
  auto lexer = JsonLexer(json_str);
  auto token = lexer.next();
  while(token)
  {
    #ifdef JSON_DEBUG_MODE
      std::cout << "Json Lexer: new token: " << token.value().type
                << ", lexer pos: " << lexer.position << "\n";
    #endif
    
    tokens.push_front(token.value());
    token = lexer.next();
  }
  
  return tokens;
}

static std::optional<Json> json_object(std::deque<JsonToken>& tokens);

static std::optional<std::vector<Json::Value>>
json_array(std::deque<JsonToken>& tokens)
{
  if (tokens.back().type != TOKEN_BEGIN_ARRAY)
    return {};
  tokens.pop_back();

  std::vector<Json::Value> array = {};

  // Allow empty arrays
  if (tokens.back().type == TOKEN_END_ARRAY)
  {
    tokens.pop_back();
    return array;
  }

  bool first = true;
  do {
    if (!first)
      tokens.pop_back();

    if (tokens.back().type == TOKEN_STRING)
    {
      array.push_back(std::get<std::string>(tokens.back().value));

      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new array value: "
                << std::get<std::string>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_INTEGER)
    {
      array.push_back(std::get<int>(tokens.back().value));
      
      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new array value: "
                << std::get<int>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_BOOLEAN)
    {
      array.push_back(std::get<bool>(tokens.back().value));
      
      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new array value: "
                << std::get<bool>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_FLOATING)
    {
      array.push_back(std::get<float>(tokens.back().value));
      
      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new array value: "
                << std::get<float>(tokens.back().value) << "\n";
      #endif
            
      tokens.pop_back();
    }

    else if (tokens.back().type == TOKEN_BEGIN_ARRAY)
    {
      auto arr = json_array(tokens).value();
      for (const auto& val : arr)
        array.push_back(val);
    }
    else if (tokens.back().type == TOKEN_OPEN_PAREN)
      array.push_back(json_object(tokens).value());
    
    first = false;
    
  } while(tokens.back().type == TOKEN_COMMA);
  
  if (tokens.back().type != TOKEN_END_ARRAY)
    return {};
  tokens.pop_back();

  return array;
}

static std::optional<Json> json_object(std::deque<JsonToken>& tokens)
{
  auto json = Json();
  if (tokens.size() == 0)
    return json;
  
  if (tokens.back().type != TOKEN_OPEN_PAREN)
    return {};
  tokens.pop_back();

  // Item
  bool first = true;
  do {
    if (!first)
      tokens.pop_back(); // pop the comma
    
    if (tokens.back().type != TOKEN_STRING)
      continue;
    
    auto key = std::get<std::string>(tokens.back().value);
    tokens.pop_back();

    if (tokens.back().type != TOKEN_COLON)
      return {};
    tokens.pop_back();

    // Value
    if (tokens.back().type == TOKEN_STRING)
    {
      json[key] = std::get<std::string>(tokens.back().value);

      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new key: " << key
                << ", value: " << std::get<std::string>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_INTEGER)
    {
      json[key] = std::get<int>(tokens.back().value);

      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new key: " << key
                << ", value: " << std::get<int>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_BOOLEAN)
    {
      json[key] = std::get<bool>(tokens.back().value);
      
      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new key: " << key
                << ", value: " << std::get<bool>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }
    else if (tokens.back().type == TOKEN_FLOATING)
    {
      json[key] = std::get<float>(tokens.back().value);

      #ifdef JSON_DEBUG_MODE
      std::cout << "Json parser, new key: " << key
                << ", value: " << std::get<float>(tokens.back().value) << "\n";
      #endif
      
      tokens.pop_back();
    }

    else if (tokens.back().type == TOKEN_BEGIN_ARRAY)
    {
      json[key] = Json::Array{ json_array(tokens).value() };
    }
    else if (tokens.back().type == TOKEN_OPEN_PAREN)
      json[key] = json_object(tokens).value();

    first = false;
    
  } while (tokens.back().type == TOKEN_COMMA);

  if (tokens.back().type != TOKEN_CLOSE_PAREN)
    return {};

  #ifdef JSON_DEBUG_MODE
  std::cout << "Json parser, success\n";
  #endif
  return json;
}

std::optional<Json> Json::parse(const std::string& json_str)
{
  auto tokens = tokenize(json_str);
  return json_object(tokens);
}
