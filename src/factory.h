
#ifndef SRC_RESTROUTEFACTORY_H_
#define SRC_RESTROUTEFACTORY_H_

#include <unordered_map>
#include <vector>
#include <functional>
using std::hash;

namespace Rest::Factory
{

  template <typename IdentifierType, class AbstractProduct>
  struct DefaultFactoryError
  {
    struct Exception : public std::exception
    {
      const char *what() const throw() { return "Unknown Type"; }
    };

    static AbstractProduct *OnUnknownType(IdentifierType)
    {
      throw Exception();
    }
  };

  template <
      class AbstractProduct,
      typename IdentifierType,
      typename ProductCreator = AbstractProduct *(*)(),
      template <typename, class> class FactoryErrorPolicy = DefaultFactoryError>
  class HttpFactory : public FactoryErrorPolicy<IdentifierType, AbstractProduct>
  {
    using IdToProductMap = std::unordered_map<
        IdentifierType,
        ProductCreator,
        std::function<std::size_t(const IdentifierType &)>,
        std::function<bool(const IdentifierType &, const IdentifierType &)>>;
    IdToProductMap associations_;

  public:
    HttpFactory(std::function<size_t(const IdentifierType &r)> hf,
                std::function<bool(const IdentifierType &r, const IdentifierType &r2)> eq)
        : associations_{150, hf, eq}
    {
    }

    ~HttpFactory()
    {
      associations_.erase(associations_.begin(), associations_.end());
    }

    bool
    Register(const IdentifierType &id, ProductCreator creator,
             std::function<size_t(const IdentifierType &r)> hf,
             std::function<bool(const IdentifierType &r, const IdentifierType &r2)> eq)
    {
      return associations_.insert({id, creator}).second != 0;
    }

    bool Unregister(const IdentifierType &id)
    {
      return associations_.erase(id) != 0;
    }

    std::vector<IdentifierType> RegisteredIds()
    {
      std::vector<IdentifierType> ids;
      for (typename IdToProductMap::iterator it = associations_.begin();
           it != associations_.end(); ++it)
      {
        ids.push_back(it->first);
      }
      return ids;
    }

    AbstractProduct *CreateObject(const IdentifierType &id)
    {
      typename IdToProductMap::iterator i = associations_.find(id);
      if (i != associations_.end())
        return (i->second)();
      return this->OnUnknownType(id);
    }
  };
}
#endif /* SRC_RESTROUTEFACTORY_H_ */