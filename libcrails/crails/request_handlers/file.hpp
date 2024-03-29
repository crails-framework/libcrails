#ifndef  FILE_REQUEST_HANDLER_HPP
# define FILE_REQUEST_HANDLER_HPP

# include "../request_handler.hpp"

namespace Crails
{
  class FileRequestHandler : public RequestHandler
  {
    typedef std::pair<unsigned int, unsigned int> Range;
  public:
    FileRequestHandler() : RequestHandler("file"), file_cache(Crails::Server::get_file_cache())
    {
#ifdef SERVER_DEBUG
      cache_enabled = false;
#else
      cache_enabled = true;
#endif
#ifdef CACHEABLE_MAX_SIZE
      cacheable_max_size = CACHEABLE_MAX_SIZE;
#else
      cacheable_max_size = 187500;
#endif
    }

    void operator()(Context&, std::function<void(bool)> callback) const override;
    bool process(Context&) const;
    bool send_file(const std::string& path, BuildingResponse& response, HttpStatus code, Range range = {0,0}) const;

    void set_cache_enabled(bool enable) { cache_enabled = enable; }
    bool is_cache_enabled(void) const   { return cache_enabled;   }

  protected:
    virtual void set_headers_for_file(BuildingResponse& response, const std::string& fullpath) const {}

  private:
    bool           cache_enabled;
    FileCache&     file_cache;
    std::uintmax_t cacheable_max_size;
  };
}
  
#endif
