#ifndef __PIE_SCANNER__
#define __PIE_SCANNER__

#include <string>

namespace pie { namespace compiler {

class Scanner {
public:
  Scanner() : m_file(stdin) {}
  Scanner(FILE *file);
  ~Scanner();

  int scan();

  FILE *file() { return m_file; }

public:
  std::string m_filename;
  int m_line;
  FILE *m_file;

private:
  void *m_yyscanner;
};

}}

#endif
