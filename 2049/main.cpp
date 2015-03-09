// MakeMakeIndexList => MMIL => 2049 => badum-tsii
#include <fstream>
using namespace std;

const int LIMIT = 32;

int main(int argc, const char * argv[]) {
	ofstream out("MakeIndexList.hpp");
	for (int i = 1; i <= LIMIT; ++i) {
		out << "constexpr static inline SizeConstantList<";
		for (int j = 0;;) {
			out << j;
			if (++j == i)
				break;
			out << ",";
		}
		out << "> MakeIndexList(SizeConstant<" << i << ">) { return {}; }" << endl;
	}
}
