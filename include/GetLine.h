#pragma once
#include <stdio.h>
class GetLine_t {
	struct Data {
		~Data() { free(line_); }
		char* line_ = nullptr;
		size_t linecap_ = 0;
	};
public:
	const char* operator()(FILE* p) const {
		/*thread_local*/ static Data data;
		ssize_t l = getline(&data.line_, &data.linecap_, p);
		if (l < 0)
			return nullptr;
		if (l > 0 && data.line_[l-1] == '\n')
			data.line_[l-1] = '\0';
		return data.line_;
	}
};
static const GetLine_t GetLine{};
