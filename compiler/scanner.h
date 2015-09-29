#ifndef __PIE_SCANNER__
#define __PIE_SCANNER__

#include <string>

namespace pie { namespace compiler {

class Scanner {
public:
  Scanner();
  ~Scanner();

  int scan();

  std::string m_filename;
  int m_line;

private:
  void *m_yyscanner;
};

}}

#endif
