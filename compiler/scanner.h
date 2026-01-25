#ifndef __PIE_SCANNER__
#define __PIE_SCANNER__

#include <string>

namespace pie { namespace compiler {

class Scanner {
public:
	Scanner() : m_file(stdin), m_line(0), m_yyscanner(nullptr) {}
	Scanner(FILE *file);
	~Scanner();

	int scan();

	FILE *file() { return m_file; }

	// Get the text of the last scanned token
	const char *tokenText() const;

	// Get the length of the last scanned token
	int tokenLength() const;

	// Get the accumulated string value (for T_STRING tokens)
	const std::string &stringValue() const;

public:
	std::string m_filename;
	int m_line;
	FILE *m_file;

	void *m_yyscanner;
};

}}

#endif
