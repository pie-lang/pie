#ifndef __PIE_SCANNER__
#define __PIE_SCANNER__

#include <string>

namespace pie { namespace compiler {

class Scanner {
public:
  void scan();

private:
  std::string m_filename;
  int m_line;

  void *m_yyscanner;
};

}}

#endif
