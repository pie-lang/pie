#ifndef __PIE_MODULE__
#define __PIE_MODULE__

#include <string>
#include <map>

using std;


namespace pie {

class Function;

class Module
{
public:
	string name;

	map <string, Function*> functions;
};

}
#endif