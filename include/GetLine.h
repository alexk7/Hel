#pragma once
#include <stdio.h>
class GetLine_t {
	struct Data {
		~Data() { free(line_); }
		char* line_ = nullptr;
		size_t linecap_ = 0;
	};
public:
	std::string operator()(FILE* p) const {
		/*thread_local*/ static Data data;
		ssize_t l = getline(&data.line_, &data.linecap_, p);
		if (l <= 0)
			return {};
		if (data.line_[l-1] == '\n')
			--l;
		return { data.line_, static_cast<size_t>(l) };
	}
};

static const GetLine_t GetLine{};
