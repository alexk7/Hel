#pragma once
template <class T, T t> struct NonTypeTemplateParameter { constexpr static auto value = t; };
