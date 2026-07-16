#pragma once

#include <cstdint>

// pointers and other types are complex and will be implemented later
enum VariantType : uint8_t {
	Void,
	Bool,
	Char,
	Short,
	Int,
	Float,
	Long,
	Double,
	Pointer
};

template<typename T> struct TypeToVariant { static_assert(sizeof(T) == 0, "unsupported type for variant"); };
template<typename T> struct TypeToVariant<const T> : TypeToVariant<T> {};
template<typename T> struct TypeToVariant<volatile T> : TypeToVariant<T> {};
template<typename T> struct TypeToVariant<const volatile T> : TypeToVariant<T> {};
template<> struct TypeToVariant<bool> { static constexpr VariantType value = Bool; };
template<> struct TypeToVariant<char> { static constexpr VariantType value = Char; };
template<> struct TypeToVariant<short> { static constexpr VariantType value = Short; };
template<> struct TypeToVariant<int> { static constexpr VariantType value = Int; };
template<> struct TypeToVariant<float> { static constexpr VariantType value = Float; };
template<> struct TypeToVariant<long long> { static constexpr VariantType value = Long; };
template<> struct TypeToVariant<double> { static constexpr VariantType value = Double; };
template<typename T> struct TypeToVariant<T*> { static constexpr VariantType value = Pointer; };
