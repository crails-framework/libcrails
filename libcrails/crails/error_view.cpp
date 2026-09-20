#include "http_response.hpp"
#include "params.hpp"
#include "request_handlers/file.hpp"
#include <crails/shared_vars.hpp>
#include <crails/renderer.hpp>
#include <crails/logger.hpp>

using namespace std;

namespace Crails
{
  static void render_custom_error_view(string view_name, BuildingResponse& response, HttpStatus code, const std::string& accept)
  {
    response.set_status_code(code);
    if (Renderer::has_renderer(view_name, accept))
    {
      SharedVars vars;
      string     content_type;

      Renderer::render(view_name, accept, response, vars);
    }
    response.send();
  }

  void render_error_view(Context& context, HttpStatus code)
  {
    const FileRequestHandler* file_handler = dynamic_cast<const FileRequestHandler*>(Server::get_request_handler("file"));
    ostringstream file_name;
    ostringstream view_name;

    context.response.set_status_code(code);
    file_name << (unsigned int)(code);
    view_name << "errors/" << file_name.str();
    if (file_handler &&
        file_handler->send_file("public/" + file_name.str() + ".html", context.response, code))
      return ;
    else if (Renderers::singleton::get() != nullptr)
    {
      const HttpRequest& request = context.connection->get_request();
      auto accept_header = request.find(HttpHeader::accept);
      auto accept = accept_header == request.end() ? string() : string(accept_header->value());

      render_custom_error_view(view_name.str(), context.response, code, accept);
    }
  }

  void render_exception_view(Context& context, string& exception_name, string& exception_message)
  {
    SharedVars  vars;
    const auto& request = context.connection->get_request();
    auto        accept_it = request.find(HttpHeader::accept);
    string_view accept_header;

    if (accept_it != request.end())
      accept_header = accept_it->value();
    vars["exception_name"] = exception_name;
    vars["exception_what"] = exception_message;
    vars["params"]         = &(context.params);
    {
      try {
        context.response.set_status_code(HttpStatus::internal_server_error);
        Renderer::render("exception", accept_header, context.response, vars);
      }
      catch (const MissingTemplate&) {
        logger << Logger::Warning << "# Exception template not found for format " << accept_header << Logger::endl;
      }
      catch (const std::exception& e) {
        logger << Logger::Error << "# Template lib/exception crashed (" << e.what() << ')' << Logger::endl;
      }
      catch (...) {
        logger << Logger::Error << "# Template lib/exception crashed" << Logger::endl;
      }
    }
  }
}
