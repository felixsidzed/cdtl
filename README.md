# cdtl
A concept for dynamic typing (or a variant type) in a compiled language inspired by Luau's TValue implementation  
Demo can be found in `main.cpp`  
For a list of supported types, see `VariantType.h`  

## Variant.h:
Standard memory-based variant type. Layout: [i64 data][i8 tag]  
Allocation: `LLVMBuildAlloca(block, Variant::makeType(), "var")`  
Storing: `var.store<T>(myValueRef)`  
Loading: `var.load<T>()`  
  
Unsafe by default, but if an assert function is passed runtime tag checks will be added. See `main.cpp`

## PackedVariant.h:
Register-based variant type packed into int128. Layout: [i64 data][i8 tag]  
Allocation: `LLVMBuildAlloca(block, PackedVariant::makeType(), "var")` or `LLVMBuildAlloca(block, LLVMInt128Type(), "var")`  
Storing: `var.store<T>(myValueRef)`  
Loading: `var.load<T>()`  
  
Unsafe by default with no tag assertion support yet.  

## RawVariant.h:
Raw register-based variant type with no type tag. Layout: [i64 data]  
Allocation: `LLVMBuildAlloca(block, RawVariant::makeType(), "var")` or `LLVMBuildAlloca(block, LLVMInt64Type(), "var")`  
Storing: `var.store<T>(myValueRef)`  
Loading: `var.load<T>()`  
  
Because it lacks a type tag the type must be assumed from external factors
