#ifndef  PARAMS_HPP
# define PARAMS_HPP

# include <crails/datatree.hpp>
# include "session_store.hpp"
# include <condition_variable>
# include <list>

namespace Crails
{
  class  Server;
  struct MultipartParser;
  class  ActionRequestHandler;
  class  BodyParser;
  class  RequestMultipartParser;

  class Params : public DataTree
  {
    friend class  Server;
    friend struct MultipartParser; 
    friend class  ActionRequestHandler;
    friend class  BodyParser;
    friend class  RequestMultipartParser;
  public:
    Params(void);
    ~Params(void);

    //
    struct File
    {
      bool operator==(const std::string& comp) const { return (key == comp); }

      std::string temporary_path;
      std::string name;
      std::string key;
      std::string mimetype;
    };

    typedef std::list<File> Files;
    //

    Data            operator[](const std::string& key)       { return (DataTree::operator[](key)); }
    const File*     operator[](const std::string& key) const { return (get_upload(key)); }
    Data            operator[](const char* key)       { return (DataTree::operator[](std::string(key))); }
    const File*     operator[](const char* key) const { return (get_upload(key)); }
    const File*     get_upload(const std::string& key) const;
    const Files&    get_files(void) const { return files; }

    Data            get_session(void) { return (session->to_data()); }

  private:
    std::unique_ptr<SessionStore> session;
    Files                         files;
  };
}

#endif
