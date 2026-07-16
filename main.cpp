// concept for dynamic typing (or a variant type) in a compiled language inspired by luau's TValue implementation
// TODO:
//     complex types

#include <iostream>
#include <Windows.h>

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>

#include <Variant.h>
#include <RawVariant.h>
#include <PackedVariant.h>

LLVMValueRef mkassert(LLVMModuleRef module) {
	LLVMTypeRef tbool = LLVMInt1Type();
	LLVMValueRef fn = LLVMAddFunction(module, "assert", LLVMFunctionType(LLVMVoidType(), &tbool, 1, 0));

	LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fn, "entry");
	LLVMBuilderRef b = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(b, entry);

	/*LLVMTypeRef fntyTrap = LLVMFunctionType(LLVMVoidType(), nullptr, 0, 0);
	LLVMValueRef trap = LLVMGetNamedFunction(module, "abort");
	if (!trap)
		trap = LLVMAddFunction(module, "abort", fntyTrap);*/

	LLVMBasicBlockRef pass = LLVMAppendBasicBlock(fn, "pass");
	LLVMBasicBlockRef fail = LLVMAppendBasicBlock(fn, "fail");
	LLVMBuildCondBr(b, LLVMGetParam(fn, 0), pass, fail);

	LLVMPositionBuilderAtEnd(b, fail);
	//LLVMBuildCall2(b, fntyTrap, trap, nullptr, 0, "");
	LLVMBuildUnreachable(b); // unreachable puts int3 iirc so for the demo we dont need to call abort

	LLVMPositionBuilderAtEnd(b, pass);
	LLVMBuildRetVoid(b);

	LLVMDisposeBuilder(b);
	return fn;
}

int main() {
	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();

	LLVMContextSetOpaquePointers(LLVMGetGlobalContext(), false); // top 10 worst features btw

	static constexpr int blahblah = 0; // qol

	LLVMModuleRef module = LLVMModuleCreateWithName("[module]");
	LLVMTypeRef charp = LLVMPointerType(LLVMInt8Type(), 0);
	LLVMValueRef fnprintf = LLVMAddFunction(module, "printf", LLVMFunctionType(LLVMVoidType(), &charp, 1, true));

	LLVMValueRef fnmain = LLVMAddFunction(module, "main", LLVMFunctionType(LLVMInt32Type(), nullptr, 0, false));
	LLVMBuilderRef block = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(block, LLVMAppendBasicBlock(fnmain, "entry"));

	// Variant
	if constexpr (blahblah == 0) {
		Variant var(LLVMBuildAlloca(block, Variant::makeType(), "pvar"), block, mkassert(module));

		LLVMValueRef args[3];
		var.store<int>(LLVMConstInt(LLVMInt32Type(), 123, false));
		args[0] = LLVMBuildGlobalString(block, "int var is %d (tt = %d)\n", "");
		args[1] = var.load<int>();
		args[2] = LLVMBuildZExt(block, var.tag(), LLVMInt32Type(), "");
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		var.store<const char*>(LLVMBuildGlobalString(block, "Hello, World!", ""));
		args[0] = LLVMBuildGlobalString(block, "char* var is %d (tt = %d)\n", "");
		args[1] = var.load<char*>();
		args[2] = LLVMBuildZExt(block, var.tag(), LLVMInt32Type(), "");
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		LLVMBuildRet(block, LLVMConstInt(LLVMInt32Type(), 0, false));
	}

	// RawVariant
	else if constexpr (blahblah == 1) {
		RawVariant var(LLVMBuildAlloca(block, RawVariant::makeType(), "pvar"), block);

		LLVMValueRef args[2];
		var.store<int>(LLVMConstInt(LLVMInt32Type(), 123, false));
		args[0] = LLVMBuildGlobalString(block, "int var is %d\n", "");
		args[1] = var.load<int>();
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		var.store<const char*>(LLVMBuildGlobalString(block, "Hello, World!", ""));
		args[0] = LLVMBuildGlobalString(block, "char* var is '%s'\n", "");
		args[1] = var.load<char*>();
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		args[0] = LLVMBuildGlobalString(block, "char* var as long is 0x%llx\n", "");
		args[1] = var.load<long long>();
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		LLVMBuildRet(block, LLVMConstInt(LLVMInt32Type(), 0, false));
	}

	// PackedVariant
	else if constexpr (blahblah == 2) {
		PackedVariant var(LLVMBuildAlloca(block, PackedVariant::makeType(), "pvar"), block);

		LLVMValueRef args[3];
		var.store<int>(LLVMConstInt(LLVMInt32Type(), 123, false));
		args[0] = LLVMBuildGlobalString(block, "int var is %d (tt = %d)\n", "");
		args[1] = var.load<int>();
		args[2] = LLVMBuildZExt(block, var.tag(), LLVMInt32Type(), "");
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		var.store<const char*>(LLVMBuildGlobalString(block, "Hello, World!", ""));
		args[0] = LLVMBuildGlobalString(block, "char* var is '%s' (tt = %d)\n", "");
		args[1] = var.load<char*>();
		args[2] = LLVMBuildZExt(block, var.tag(), LLVMInt32Type(), "");
		LLVMBuildCall(block, fnprintf, args, _countof(args), "");

		LLVMBuildRet(block, LLVMConstInt(LLVMInt32Type(), 0, false));
	}

	LLVMDisposeBuilder(block);
	LLVMDumpModule(module);
	putchar('\n');

	char* err;
	LLVMExecutionEngineRef ee;
	if (LLVMCreateExecutionEngineForModule(&ee, module, &err)) {
		printf("bad: %s\n", err);
		LLVMDisposeMessage(err);
		return 1;
	}

	((int(*)())LLVMGetFunctionAddress(ee, "main"))();
	//printf("\n\nmain returned: %d\n", ((int(*)())LLVMGetFunctionAddress(ee, "main"))());

	LLVMDisposeModule(module);
	return 0;
}
