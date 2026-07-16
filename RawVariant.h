#pragma once

#include <VariantType.h>

#include <llvm-c/Core.h>

// register based raw variant with no type tag
// for a register based variant with a type tag, see SSEVariant
struct RawVariant {
	LLVMValueRef self;
	LLVMBuilderRef block;

	RawVariant(LLVMValueRef self, LLVMBuilderRef block) : self(self), block(block) {}

	static LLVMTypeRef makeType() {
		return LLVMInt64Type();
	}

	template<typename T>
	void store(LLVMValueRef v) const {
		constexpr auto tt = TypeToVariant<T>::value;

		if constexpr (tt == VariantType::Pointer)
			LLVMBuildStore(block, LLVMBuildPtrToInt(block, v, LLVMInt64Type(), ""), self);
		else
			LLVMBuildStore(block, LLVMBuildZExtOrBitCast(block, v, LLVMInt64Type(), ""), self);
	}

	template<typename T>
	LLVMValueRef load() const {
		constexpr auto tt = TypeToVariant<T>::value;

		LLVMValueRef raw = LLVMBuildLoad2(block, LLVMInt64Type(), self, "");
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
