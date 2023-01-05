#ifndef  CRAILS_BUILTIN_ASSETS_HPP
# define CRAILS_BUILTIN_ASSETS_HPP

# include <crails/request_handler.hpp>
# include <crails/mimetype.hpp>
# include <map>
# include <string_view>

namespace Crails
{
  struct BuiltinAsset
  {
    const char* data;
    size_t      length;
    std::string mimetype;
  };

  class BuiltinAssets : public std::map<std::string_view, BuiltinAsset>
  {
    std::string_view path;
    std::string_view compression_strategy;
  public:
    BuiltinAssets(std::string_view path, std::string_view compression_strategy) : path(path), compression_strategy(compression_strategy)
    {
    }

    bool accepts(const HttpRequest& request) const;

    void add(const std::string_view name, const char* data, size_t length);

    std::string_view get_root_path() const { return path; }
    std::string_view get_content_encoding() const { return compression_strategy; }
  };

  class BuiltinAssetsHandler : public Crails::RequestHandler
  {
  public:
    BuiltinAssetsHandler(const BuiltinAssets& library);

    void operator()(Context& context, std::function<void(bool)> callback) const override;

  private:
    bool process(const std::string& path, BuildingResponse& response) const;

    const std::string_view path;
    const BuiltinAssets&   library;
  };
}

#endif
