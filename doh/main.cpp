#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <regex>
#include <set>
#include <unordered_set>
#include <vector>
#include "GetLine.h"
#include "MakeUniquePtr.h"
#include "Mismatch.h"
#include "StartsWith.h"
#include "With.h"
using namespace std;

int main(int argc, const char * argv[]) {
	auto filenames = With(MakeUniquePtr(popen("ls *.h", "r"), pclose), [](auto pFile) {
		vector<string> v;
		while (auto line = GetLine(pFile.get()))
			v.emplace_back(line);
		return v;
	});
	unordered_set<string> symbols;
	for (const auto& filename : filenames)
		symbols.insert(filename.substr(0, filename.rfind('.')));
	for (const auto& filename : filenames) {
		if (auto pFile = MakeUniquePtr(fopen(filename.c_str(), "r"), fclose)) {
			cout << "FILE: " << filename << endl;
			set<string> dependencies;
			vector<string> code;
			while (auto pLine = GetLine(pFile.get())) {
				string line{pLine};
				if (!line.empty() && line != "#pragma once" && !StartsWith(line, "#include \""s)) {
					static const regex r{"[_a-zA-Z][_a-zA-Z0-9]*"};
					sregex_iterator i{begin(line), end(line), r}, iEnd;
					for_each(i, iEnd, [&](auto m) {
						string s = m.str();
						if (symbols.find(s) != symbols.end() && s + ".h" != filename)
							dependencies.insert(s);
					});
					code.push_back(line);
				}
			}
			ofstream out(filename);
			out << "#pragma once\n";
			for (const auto& e : dependencies)
				out << "#include \"" << e << ".h\"\n";
			for (const auto& e : code)
				out << e << endl;
		}
	}
}
