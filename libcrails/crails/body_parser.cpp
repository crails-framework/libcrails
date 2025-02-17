#include "request_parser.hpp"
#include "context.hpp"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Crails;

BodyParser::PendingBody::PendingBody(Context& context)
  : connection(*context.connection), out(context.response), params(context.params), total_read(0)
{
  auto& parser = connection.get_parser();
  auto content_length = parser.content_length();
  auto remaining = parser.content_length_remaining();

  to_read    = content_length ? *content_length      : 0;
  total_read = remaining      ? to_read - *remaining : 0;
}

void BodyParser::PendingBody::on_received_chunk(string_view chunk)
{
  size_t previous_length = read_buffer.length();

  read_buffer += chunk;
  total_read += chunk.length();
  if (total_read >= to_read && finished_callback)
    finished_callback();
}

void BodyParser::body_too_large(Context& context) const
{
  context.response.set_status_code(HttpStatus::payload_too_large);
  context.response.send();
}

void BodyParser::wait_for_body(Context& context, function<void()> finished_callback) const
{
  shared_ptr<PendingBody> pending_body = make_shared<PendingBody>(context);
  auto callback = [this, &context, pending_body, finished_callback]()
  {
    body_received(context, pending_body->read_buffer);
    finished_callback();
    pending_body->finished_callback = function<void()>();
  };

  if (pending_body->total_read >= pending_body->to_read)
    callback();
  else if (pending_body->to_read > pending_body->read_buffer.max_size())
  {
    body_too_large(context);
    finished_callback();
  }
  else
  {
    pending_body->read_buffer.reserve(pending_body->to_read);
    pending_body->finished_callback = callback;
    context.connection->on_received_body_chunk(
      std::bind(
        &BodyParser::PendingBody::on_received_chunk,
        pending_body,
        std::placeholders::_1
      )
    );
  }
}
