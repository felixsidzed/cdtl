// concept for dynamic typing (or a variant type) in a compiled language inspired by luau's TValue implementation
// TODO:
//     complex types

#include <iostream>
#include <Windows.h>

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>

// pointers and other types are complex and will be implemented later
enum VariantType : uint8_t {
	Void,
	Char,
	Short,
	Int,
	Long,
	String // temporary (until complex types are implemented)
};

template<typename T>
struct TypeToVariant { static constexpr VariantType value = TypeToVariant<std::remove_cv_t<T>>::value; };
template<>
struct TypeToVariant<char> { static constexpr VariantType value = Char; };
template<>
struct TypeToVariant<short> { static constexpr VariantType value = Short; };
template<>
struct TypeToVariant<int> { static constexpr VariantType value = Int; };
template<>
struct TypeToVariant<long> { static constexpr VariantType value = Long; };
template<>
struct TypeToVariant<const char*> { static constexpr VariantType value = String; };

// wrapper
// TODO: maybe add support for when self isnt a pointer
// TODO: add optional runtime checks
struct Variant {
	LLVMValueRef self;
	LLVMBuilderRef block;

	Variant(LLVMValueRef self, LLVMBuilderRef block) : self(self), block(block) {};

	// TODO: maybe use templates
	LLVMValueRef asChar() { return LLVMBuildLoad(block, LLVMBuildBitCast(block, self, LLVMPointerType(LLVMInt8Type(), 0), ""), ""); }
	LLVMValueRef asShort() { return LLVMBuildLoad(block, LLVMBuildBitCast(block, self, LLVMPointerType(LLVMInt16Type(), 0), ""), ""); }
	LLVMValueRef asInt() { return LLVMBuildLoad(block, LLVMBuildBitCast(block, self, LLVMPointerType(LLVMInt32Type(), 0), ""), ""); }
	LLVMValueRef asLong() { return LLVMBuildLoad(block, LLVMBuildBitCast(block, self, LLVMPointerType(LLVMInt64Type(), 0), ""), ""); }
	LLVMValueRef asString() { return LLVMBuildLoad(block, LLVMBuildBitCast(block, self, LLVMPointerType(LLVMInt8Type(), 0), ""), ""); }

	LLVMValueRef tt() { return LLVMBuildLoad(block, LLVMBuildStructGEP(block, self, 1, ""), ""); }
	
	template<typename T>
	void store(LLVMValueRef value) {
		static_assert(TypeToVariant<T>::value != VariantType::Void, "Can't convert to variant type");
		LLVMBuildStore(block, LLVMConstInt(LLVMInt8Type(), TypeToVariant<T>::value, false), LLVMBuildStructGEP(block, self, 1, ""));
		LLVMBuildStore(block, value, self);
	}
};

int main() {
	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();

	LLVMContextSetOpaquePointers(LLVMGetGlobalContext(), false); // top 10 worst features btw

	LLVMModuleRef module = LLVMModuleCreateWithName("[module]");
	{
		LLVMTypeRef variant = LLVMStructCreateNamed(LLVMGetGlobalContext(), "variant");

		//struct variant {
		//	union {
		//		char c;
		//		short h;
		//		int d;
		//		long long ll;
		//		char* s; // temporary (until complex types are implemented)
		//	};
		//	VariantType tt;
		//};
		LLVMTypeRef body[] = { LLVMInt64Type(), LLVMInt8Type() };
		LLVMStructSetBody(variant, body, _countof(body), false);

		LLVMTypeRef charp = LLVMPointerType(LLVMInt8Type(), 0);
		LLVMValueRef fnprintf = LLVMAddFunction(module, "printf", LLVMFunctionType(LLVMVoidType(), &charp, 1, true));

		LLVMValueRef fnmain = LLVMAddFunction(module, "main", LLVMFunctionType(LLVMInt32Type(), nullptr, 0, false));
		LLVMBuilderRef block = LLVMCreateBuilder();
		LLVMPositionBuilderAtEnd(block, LLVMAppendBasicBlock(fnmain, "entry"));

		Variant var(LLVMBuildAlloca(block, variant, "varPtr"), block);

		var.store<int>(LLVMConstInt(LLVMInt32Type(), 67, false));
		
		{
			LLVMValueRef args[] = {
				LLVMBuildGlobalString(block, "var as an integer is %d (tt = %d)\n", ""),
				var.asInt(), LLVMBuildZExt(block, var.tt(), LLVMInt32Type(), "")
			};
			LLVMBuildCall(block, fnprintf, args, _countof(args), "");
		}

		var.store<const char*>(LLVMBuildGlobalString(block, "Hello, World!", ""));

		{
			LLVMValueRef args[] = {
				LLVMBuildGlobalString(block, "var as a string is '%s' (tt = %d)\n", ""),
				var.asString(), LLVMBuildZExt(block, var.tt(), LLVMInt32Type(), "")
			};
			LLVMBuildCall(block, fnprintf, args, _countof(args), "");
		}

		LLVMBuildRet(block, LLVMConstInt(LLVMInt32Type(), 0, false));

		LLVMDisposeBuilder(block);
	}

	LLVMDumpModule(module);
	putchar('\n');

	char* err;
	LLVMExecutionEngineRef ee;
	if (LLVMCreateExecutionEngineForModule(&ee, module, &err)) {
		printf("bad: %s\n", err);
		LLVMDisposeMessage(err);
		return 1;
	}

	printf("\n\nmain returned: %d\n", ((int(*)())LLVMGetFunctionAddress(ee, "main"))());

	LLVMDisposeModule(module);
	return 0;
}
