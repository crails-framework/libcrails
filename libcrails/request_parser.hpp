#ifndef  REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP

# include "server/connection.hpp"
# include <regex>
# include <functional>

namespace Crails
{
  class Context;
  class BuildingResponse;
  class Params;

  class RequestParser
  {
  public:
    enum Status
    {
      Continue,
      Stop,
      Abort
    };

    virtual ~RequestParser() {}
    
    virtual void operator()(Context&, std::function<void(Status)>) const = 0;
  protected:
    bool content_type_matches(const HttpRequest&, const std::regex) const;
  };

  class BodyParser : public RequestParser
  {
  public:
    void wait_for_body(Context&, std::function<void()>) const;
  protected:
    virtual void body_received(Context&, const std::string& body) const = 0;
  private:
    struct PendingBody
    {
      PendingBody(Context&);

      Connection&           connection;
      BuildingResponse&     out;
      Params&               params;
      std::string           read_buffer;
      unsigned int          total_read;
      unsigned int          to_read;
      std::function<void()> finished_callback;
    };
  };
}

#endif
