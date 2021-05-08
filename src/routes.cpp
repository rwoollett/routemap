#include "factory.h"
#include <iostream>
#include <unordered_set>
#include <memory>

using std::hash;
using std::string;

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

// namespace std {
//   template<>
//   struct hash<Record
// }

size_t hf(const RoutePath &r) { return hash<string>()(r.method) ^ hash<string>()(r.path); };
bool eq(const RoutePath &r, const RoutePath &r2) { return r.method == r2.method && r.path == r2.path; };

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

  // std::cerr << " decl " << std::string(typeid(decltype(&hf)).name()) << "\n";

  // std::unordered_map<RoutePath, int, decltype(&hf), decltype(&eq)> IdToProductMap{10, hf, eq};
  // IdToProductMap.insert({{"GET", path}, 2000});
  // std::unordered_map<RoutePath, int, decltype(&hf), decltype(&eq)>::iterator i = IdToProductMap.find({"GET", path});
  // printf("found %d\n", (*i).second);

  RouteFactory factory(&hf, &eq);

  for (int i = 0; i < 100; i++)
  {
    factory.Register({"GET", std::string(path + std::to_string(i))}, createRoute, hf, eq);
  };

  std::vector<std::unique_ptr<Route>> routes{};
  for (int i = 0; i < 100; i++)
  {
    std::unique_ptr<Route> route = std::unique_ptr<Route>(factory.CreateObject({"GET", std::string(path + std::to_string(i))}));
    if (route)
    {
      route->set_path(path + std::to_string(i));
      routes.push_back(std::move(route));
    }
    else
    {
      printf("Route not created %d\n",i);
    }
  };

  for (auto& r: routes)
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
