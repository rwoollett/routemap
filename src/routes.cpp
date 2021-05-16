#include "factory.h"
#include <iostream>
#include <unordered_set>
#include <memory>
#include <sstream>

using std::hash;
using std::string;

// class RouteHandler {
//   std::string
//   std::vector<std::string> m_statics{};
//   public:
//   RouteHandler(const std::string path) :

// };

class Route
{
  std::string m_path{};

public:
  Route(){};

  inline void set_path(const std::string &path)
  {
    m_path = path;
  };
  inline string path()
  {
    return m_path;
  };
};

struct RoutePath
{
  string method;
  string path;
};

std::pair<std::string::const_iterator, std::string> segmont_variable(std::string::const_iterator i,
                                                                     std::string::const_iterator e)
{
  std::string variable{};
  std::stringstream ss{};

  if (*i != '{')
  {
    ss << "not valid in variable segmont " << *i;
    throw std::runtime_error(ss.str());
  }
  i++;
  if (!isalpha(*i))
  {
    ss << "not valid in variable segmont " << *i;
    throw std::runtime_error(ss.str());
  }
  while (!(i == e || *i == '}'))
  {
    if (!isalnum(*i))
    {
      ss << "not valid in variable segmont " << *i;
      throw std::runtime_error(ss.str());
    }
    variable.push_back(*i);
    i++;
  }
  if (i == e)
  {
    ss << "not valid with missing } in variable segmont";
    throw std::runtime_error(ss.str());
  }
  //std::cerr << *i;
  i++;
  return {i, variable};
}

std::pair<std::string::const_iterator, std::string> segmont_static(std::string::const_iterator i,
                                                                   std::string::const_iterator e)
{
  std::string static_segmont{};
  std::stringstream ss{};
  if (!isalpha(*i))
  {
    ss << "not valid in segmont " << *i;
    throw std::runtime_error(ss.str());
  }
  while (!(i == e || *i == '/'))
  {
    if (!isalnum(*i))
    {
      ss << "not valid in segmont " << *i;
      throw std::runtime_error(ss.str());
    }
    static_segmont.push_back(*i);
    i++;
  }
  return {i, static_segmont};
}

std::pair<std::string, std::string> route_name(const std::string &path)
{
  std::string::const_iterator i = path.cbegin();
  std::vector<std::string> static_segmont{};
  std::vector<std::string> variable_segmont{};
  std::stringstream ss{};
  int var_count = 0;
  try
  {
    while (i != path.cend())
    { // level root
      if (*i == '/')
      {
        i++;
        // level in a /
        if (*i == '{')
        {
          auto res = segmont_variable(i, path.cend());
          variable_segmont.push_back(res.second);
          i = res.first;
          var_count++;
        }
        else if (i == path.cend())
          break;
        else
        {
          if (var_count)
          {
            ss << "not allowed variables before static segmonts";
            throw std::runtime_error(ss.str());
          }
          auto res = segmont_static(i, path.cend());
          static_segmont.push_back(res.second);
          i = res.first;
        }
      }
      else if (*i == '{')
      {
        ss << "not allowed multiple variables inside / separators";
        throw std::runtime_error(ss.str());
      }
      else
        i++;
    }
  }
  catch (std::exception &e)
  {
    throw;
  }
  std::string route_name{};
  for (auto s : static_segmont)
  {
    route_name += '/' + s;
  }
  std::string variables{};
  for (auto s : variable_segmont)
  {
    variables += '{'+s+"} ";
  }
  return {route_name, variables};
}

size_t hf(const RoutePath &r) { return hash<string>()(r.method) ^ hash<string>()(r.path); };
// bool eq(const RoutePath &r, const RoutePath &r2) { return r.method == r2.method && r.path == r2.path; };

using RouteFactory = Rest::Factory::HttpFactory<Route, RoutePath>; //, decltype(&hf), decltype(&eq)>;
Route *createRoute()
{
  return new Route();
}

int main(int argc, char **argv)
{
  string path{};
  if (argc > 1)
  {
    printf("Path: %s\n", argv[1]);
    path = argv[1];
  }

  RouteFactory factory(
      30,
      &hf,
      // [](const RoutePath &r) {
      //   return hash<string>()(r.method) ^ hash<string>()(r.path);
      // },
      [](const RoutePath &r, const RoutePath &r2) {
        return r.method == r2.method && r.path == r2.path;
      });

  // Route handler for url
  // /person/staff  - static urls
  // /person/{id}   - static of one or more /<static>  ie /person/one/two has three static parts
  // The combined statics will be used to register the static url and http verb
  //   RoutePath{ url: <static> whole url containing: "/person/one/two", method "GET"}

  // The {id} sections in url are used to create url params with name="id" and value equal the url value
  // /person/me  will create name/value pair ("id" = "me")
  // /person/3   "id" = "3"
  //
  try
  {
    auto res = route_name(path);
    std::cerr << "Route name: " << res.first << " " << res.second << "\n";
  }
  catch (std::exception &e)
  {
    std::cerr << "Except: " << e.what() << "\n";
  }
  exit(1);

  for (int i = 0; i < 100; i++)
  {
    factory.Register({"GET", std::string(path + std::to_string(i))}, createRoute);

    // printf("Hash is : %ld  %s %s\n",
    //        hf({"GET", std::string(path + std::to_string(i))}),
    //        "GET",
    //        std::string(path + std::to_string(i)).c_str());
    // printf("Hash is : %ld  %s %s\n\n",
    //        hf({"GET", std::string(path + std::to_string(i))}),
    //        "GET",
    //        std::string(path + std::to_string(i)).c_str());
  };

  std::vector<std::unique_ptr<Route>> routes{};
  for (int i = 0; i < 100; i++)
  {
    std::unique_ptr<Route> route = std::unique_ptr<Route>(
        factory.CreateObject({"GET", std::string(path + std::to_string(i))}));
    if (route)
    {
      route->set_path(path + std::to_string(i));
      routes.push_back(std::move(route));
    }
    else
    {
      printf("Route not created %d\n", i);
    }
  };

  for (auto &r : routes)
  {
    if (r)
    {
      printf("Route created: %s\n", r->path().c_str());
    }
    else
    {
      printf("R not found");
    }
  };

  return 0;
}
