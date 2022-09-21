#include "request_parser.hpp"
#include "context.hpp"
#include <crails/logger.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Crails;

BodyParser::PendingBody::PendingBody(Context& context)
  : connection(*context.connection), out(context.response), params(context.params), total_read(0)
{
  const char* sizeHeader = "Content-Length";
  const HttpRequest& request = connection.get_request();

  if (request.find(sizeHeader) == request.end())
    to_read = 0;
  else
    to_read = boost::lexical_cast<unsigned int>(request[sizeHeader]);
  total_read = request.body().size();
}

void BodyParser::wait_for_body(Context& context, function<void()> finished_callback) const
{
  shared_ptr<PendingBody> pending_body = make_shared<PendingBody>(context);

  if (pending_body->total_read >= pending_body->to_read)
  {
    body_received(context, context.connection->get_request().body());
    finished_callback();
  }
  else
    throw std::runtime_error("BoddyParser: Asynchronous body reception not implemented");
}
