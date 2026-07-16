#pragma once

#include <VariantType.h>

#include <llvm-c/Core.h>

// register based variant with a type tag
// stores tag + payload in a single 128 bit value
struct PackedVariant {
	LLVMValueRef self;
	LLVMBuilderRef block;

	PackedVariant(LLVMValueRef self, LLVMBuilderRef block) : self(self), block(block) {}

	static LLVMTypeRef makeType() {
		return LLVMInt128Type();
	}

	LLVMValueRef tag() const {
		return LLVMBuildTrunc(block,
			LLVMBuildLShr(block,
				LLVMBuildLoad2(block, LLVMInt128Type(), self, ""),
				LLVMConstInt(LLVMInt128Type(), 64, false), ""
			),
			LLVMInt8Type(), ""
		);
	}

	LLVMValueRef data() const {
		return LLVMBuildTrunc(block, LLVMBuildLoad2(block, LLVMInt128Type(), self, ""), LLVMInt64Type(), "");
	}
	
	template<typename T>
	void store(LLVMValueRef v) const {
		constexpr auto tt = TypeToVariant<T>::value;

		LLVMValueRef raw;
		if constexpr (tt == VariantType::Pointer)
			raw = LLVMBuildPtrToInt(block, v, LLVMInt64Type(), "");
		else
			raw = LLVMBuildZExtOrBitCast(block, v, LLVMInt64Type(), "");

		// TODO: maybe make the shifting compile time? 
		// tho msvc doesnt support int128 and it prob gets optimized away 
		LLVMBuildStore(block, LLVMBuildOr(block,
			LLVMBuildShl(block,
				LLVMBuildZExt(block, LLVMConstInt(LLVMInt8Type(), tt, false), LLVMInt128Type(), ""),
				LLVMConstInt(LLVMInt128Type(), 64, false), ""
			),
			LLVMBuildZExt(block, raw, LLVMInt128Type(), ""), ""
		), self);
	}

	template<typename T>
	LLVMValueRef load() const {
		constexpr auto tt = TypeToVariant<T>::value;

		LLVMValueRef raw = LLVMBuildTrunc(block, LLVMBuildLoad2(block, LLVMInt128Type(), self, ""), LLVMInt64Type(), "");

		if constexpr (tt == VariantType::Pointer)
			return LLVMBuildIntToPtr(block, raw, LLVMPointerType(LLVMInt8Type(), 0), "");
		else {
			LLVMTypeRef target;

			if constexpr (tt == VariantType::Bool) target = LLVMInt1Type();
			else if constexpr (tt == VariantType::Char) target = LLVMInt8Type();
			else if constexpr (tt == VariantType::Short) target = LLVMInt16Type();
			else if constexpr (tt == VariantType::Int) target = LLVMInt32Type();
			else if constexpr (tt == VariantType::Long) target = LLVMInt64Type();
			else if constexpr (tt == VariantType::Float) target = LLVMFloatType();
			else if constexpr (tt == VariantType::Double) target = LLVMDoubleType();

			return LLVMBuildTruncOrBitCast(block, raw, target, "");
		}
	}
};
