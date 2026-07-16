#pragma once

#include <VariantType.h>

#include <llvm-c/Core.h>

// standard memor based variant unsafe wrapper
// if fnAssert is passed, load will have safety checks
struct Variant {
	LLVMValueRef self;
	LLVMBuilderRef block;
	LLVMValueRef fnAssert;

	Variant(LLVMValueRef self, LLVMBuilderRef block, LLVMValueRef fnAssert = nullptr)
		: self(self), block(block), fnAssert(fnAssert) {}

	static LLVMTypeRef makeType() {
		LLVMTypeRef ty = LLVMStructCreateNamed(LLVMGetGlobalContext(), "Variant_t");
		LLVMTypeRef body[] = { LLVMInt64Type(), LLVMInt8Type() };
		LLVMStructSetBody(ty, body, _countof(body), false);
		return ty;
	}

	LLVMValueRef ptag() const { return LLVMBuildStructGEP(block, self, 1, ""); }
	LLVMValueRef pdata() const { return LLVMBuildStructGEP(block, self, 0, ""); }
	LLVMValueRef tag() const { return LLVMBuildLoad2(block, LLVMInt8Type(), ptag(), ""); }

	template<typename T>
	void store(LLVMValueRef v) const {
		constexpr auto tt = TypeToVariant<T>::value;

		LLVMBuildStore(block, LLVMConstInt(LLVMInt8Type(), (uint8_t)tt, false), ptag());

		if constexpr (tt == VariantType::Pointer)
			LLVMBuildStore(block, LLVMBuildPtrToInt(block, v, LLVMInt64Type(), ""), pdata());
		else
			LLVMBuildStore(block, LLVMBuildZExtOrBitCast(block, v, LLVMInt64Type(), ""), pdata());
	}

	template<typename T>
	LLVMValueRef load() const {
		constexpr auto tt = TypeToVariant<T>::value;

		LLVMValueRef curtag = tag();
		if (fnAssert) {
			LLVMValueRef param = LLVMBuildICmp(block, LLVMIntEQ, curtag, LLVMConstInt(LLVMInt8Type(), tt, false), "");
			LLVMBuildCall(block, fnAssert, &param, 1, "");
		}

		LLVMValueRef raw = LLVMBuildLoad2(block, LLVMInt64Type(), pdata(), "");

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
