#include "test.h"
#include <parser.h>
#include <ast.h>
#include <hir.h>
#include <hir-inl.h>

TEST_START(hir)
  HIR_TEST("return 1 + 2\n",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[1]\n"
           "i4 = Literal[2]\n"
           "i6 = BinOp(i2, i4)\n"
           "i8 = Return(i6)\n")

  // Simple assignments
  HIR_TEST("a = 1\nb = 1\nreturn a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[1]\n"
           "i4 = Literal[1]\n"
           "i6 = Return(i2)\n")
  HIR_TEST("return { a: 1 }",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = AllocateObject\n"
           "i4 = Literal[1]\n"
           "i6 = Literal[a]\n"
           "i8 = StoreProperty(i2, i6, i4)\n"
           "i10 = Return(i2)\n")
  HIR_TEST("return ['a']",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = AllocateArray\n"
           "i4 = Literal[0]\n"
           "i6 = Literal[a]\n"
           "i8 = StoreProperty(i2, i4, i6)\n"
           "i10 = Return(i2)\n")
  HIR_TEST("a = {}\na.b = 1\ndelete a.b\nreturn a.b",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = AllocateObject\n"
           "i4 = Literal[1]\n"
           "i6 = Literal[b]\n"
           "i8 = StoreProperty(i2, i6, i4)\n"
           "i10 = Literal[b]\n"
           "i12 = DeleteProperty(i2, i10)\n"
           "i14 = Nil\n"
           "i16 = Literal[b]\n"
           "i18 = LoadProperty(i2, i16)\n"
           "i20 = Return(i18)\n")
  HIR_TEST("a = global\nreturn a:b(1,2)",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = LoadContext\n"
           "i4 = Literal[1]\n"
           "i8 = Literal[2]\n"
           "i12 = Literal[3]\n"
           "i16 = Literal[b]\n"
           "i18 = LoadProperty(i2, i16)\n"
           "i20 = AlignStack(i12)\n"
           "i14 = StoreArg(i2)\n"
           "i10 = StoreArg(i8)\n"
           "i6 = StoreArg(i4)\n"
           "i22 = Call(i18, i12)\n"
           "i24 = Return(i22)\n")

  // Var arg
  HIR_TEST("fn(a, b..., c) { return a + b[0] + b[1] + c }\n"
           "return fn(1, 2, [3,4]...)",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Function[b1]\n"
           "i4 = Literal[1]\n"
           "i8 = Literal[2]\n"
           "i12 = AllocateArray\n"
           "i14 = Literal[0]\n"
           "i16 = Literal[3]\n"
           "i18 = StoreProperty(i12, i14, i16)\n"
           "i20 = Literal[1]\n"
           "i22 = Literal[4]\n"
           "i24 = StoreProperty(i12, i20, i22)\n"
           "i28 = Literal[2]\n"
           "i30 = Sizeof(i12)\n"
           "i32 = BinOp(i28, i30)\n"
           "i34 = AlignStack(i32)\n"
           "i26 = StoreVarArg(i12)\n"
           "i10 = StoreArg(i8)\n"
           "i6 = StoreArg(i4)\n"
           "i36 = Call(i2, i32)\n"
           "i38 = Return(i36)\n"
           "# Block 1\n"
           "i40 = Entry[0]\n"
           "i42 = Literal[0]\n"
           "i44 = LoadArg(i42)\n"
           "i46 = Literal[1]\n"
           "i48 = LoadVarArg(i46)\n"
           "i50 = Sizeof(i48)\n"
           "i52 = BinOp(i46, i50)\n"
           "i54 = LoadArg(i52)\n"
           "i56 = Literal[0]\n"
           "i58 = LoadProperty(i48, i56)\n"
           "i60 = Literal[1]\n"
           "i62 = LoadProperty(i48, i60)\n"
           "i64 = BinOp(i62, i54)\n"
           "i66 = BinOp(i58, i64)\n"
           "i68 = BinOp(i44, i66)\n"
           "i70 = Return(i68)\n")

  // Unary operations
  HIR_TEST("i = 0\nreturn !i",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[0]\n"
           "i4 = Not(i2)\n"
           "i6 = Return(i4)\n")
  HIR_TEST("i = 1\nreturn +i",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[1]\n"
           "i4 = Literal[0]\n"
           "i6 = BinOp(i4, i2)\n"
           "i8 = Return(i6)\n")
  HIR_TEST("i = 0\nreturn ++i",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[0]\n"
           "i4 = Literal[1]\n"
           "i6 = BinOp(i2, i4)\n"
           "i8 = Return(i6)\n")
  HIR_TEST("i = 0\nreturn i++",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[0]\n"
           "i4 = Literal[1]\n"
           "i6 = BinOp(i2, i4)\n"
           "i8 = Return(i2)\n")

  // Logical operations
  HIR_TEST("i = 0\nreturn i && 1",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[0]\n"
           "i4 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 1\n"
           "i6 = If(i2)\n"
           "# succ: 2 3\n"
           "--------\n"
           "# Block 2\n"
           "i8 = Literal[1]\n"
           "i10 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 3\n"
           "i12 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 4\n"
           "i14 = Phi(i8, i2)\n"
           "i16 = Return(i14)\n")
  HIR_TEST("i = 0\nreturn i || 1",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[0]\n"
           "i4 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 1\n"
           "i6 = If(i2)\n"
           "# succ: 2 3\n"
           "--------\n"
           "# Block 2\n"
           "i10 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 3\n"
           "i8 = Literal[1]\n"
           "i12 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 4\n"
           "i14 = Phi(i2, i8)\n"
           "i16 = Return(i14)\n")

  // Multiple blocks and phi
  HIR_TEST("if (a) { a = 2 }\nreturn a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Nil\n"
           "i4 = If(i2)\n"
           "# succ: 1 2\n"
           "--------\n"
           "# Block 1\n"
           "i6 = Literal[2]\n"
           "i8 = Goto\n"
           "# succ: 3\n"
           "--------\n"
           "# Block 2\n"
           "i10 = Goto\n"
           "# succ: 3\n"
           "--------\n"
           "# Block 3\n"
           "i12 = Phi(i6, i2)\n"
           "i14 = Return(i12)\n")

  HIR_TEST("if (a) { a = 2 } else { a = 3 }\nreturn a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Nil\n"
           "i4 = If(i2)\n"
           "# succ: 1 2\n"
           "--------\n"
           "# Block 1\n"
           "i6 = Literal[2]\n"
           "i10 = Goto\n"
           "# succ: 3\n"
           "--------\n"
           "# Block 2\n"
           "i8 = Literal[3]\n"
           "i12 = Goto\n"
           "# succ: 3\n"
           "--------\n"
           "# Block 3\n"
           "i14 = Phi(i6, i8)\n"
           "i16 = Return(i14)\n")

  HIR_TEST("a = 1\nif (a) {\n" 
           "  a = 2\n"
           "} else {\n"
           "  if (a) {\n"
           "    if (a) {\n"
           "      a = 3\n"
           "    }\n"
           "  } else {\n"
           "    a = 4\n"
           "  }\n"
           "}\n"
           "return a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[1]\n"
           "i4 = If(i2)\n"
           "# succ: 1 2\n"
           "--------\n"
           "# Block 1\n"
           "i6 = Literal[2]\n"
           "i32 = Goto\n"
           "# succ: 9\n"
           "--------\n"
           "# Block 2\n"
           "i10 = If(i2)\n"
           "# succ: 3 4\n"
           "--------\n"
           "# Block 3\n"
           "i14 = If(i2)\n"
           "# succ: 5 6\n"
           "--------\n"
           "# Block 4\n"
           "i24 = Literal[4]\n"
           "i28 = Goto\n"
           "# succ: 8\n"
           "--------\n"
           "# Block 5\n"
           "i16 = Literal[3]\n"
           "i18 = Goto\n"
           "# succ: 7\n"
           "--------\n"
           "# Block 6\n"
           "i20 = Goto\n"
           "# succ: 7\n"
           "--------\n"
           "# Block 7\n"
           "i22 = Phi(i16, i2)\n"
           "i26 = Goto\n"
           "# succ: 8\n"
           "--------\n"
           "# Block 8\n"
           "i30 = Phi(i22, i24)\n"
           "i34 = Goto\n"
           "# succ: 9\n"
           "--------\n"
           "# Block 9\n"
           "i36 = Phi(i6, i30)\n"
           "i38 = Return(i36)\n")

  // While loop
  HIR_TEST("while (true) { a++ }\nreturn a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Nil\n"
           "i4 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 1 (loop)\n"
           "i6 = Phi(i2, i18)\n"
           "i8 = Goto\n"
           "# succ: 2\n"
           "--------\n"
           "# Block 2\n"
           "i10 = Literal[true]\n"
           "i12 = If(i10)\n"
           "# succ: 3 5\n"
           "--------\n"
           "# Block 3\n"
           "i14 = Literal[1]\n"
           "i18 = BinOp(i6, i14)\n"
           "i20 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 4\n"
           "i22 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 5\n"
           "i24 = Goto\n"
           "# succ: 6\n"
           "--------\n"
           "# Block 6\n"
           "i28 = Return(i6)\n")

  // Break, continue
  HIR_TEST("a = 1\n"
           "while(nil) {\n"
           "  a = 2\n"
           "  if (true) { continue }\n"
           "  a = 3\n"
           "}\n"
           "return a",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[1]\n"
           "i4 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 1 (loop)\n"
           "i6 = Phi(i2, i30)\n"
           "i8 = Goto\n"
           "# succ: 2\n"
           "--------\n"
           "# Block 2\n"
           "i10 = Nil\n"
           "i12 = If(i10)\n"
           "# succ: 3 5\n"
           "--------\n"
           "# Block 3\n"
           "i14 = Literal[2]\n"
           "i16 = Literal[true]\n"
           "i18 = If(i16)\n"
           "# succ: 6 7\n"
           "--------\n"
           "# Block 4\n"
           "i34 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 5\n"
           "i36 = Goto\n"
           "# succ: 10\n"
           "--------\n"
           "# Block 6\n"
           "i20 = Goto\n"
           "# succ: 8\n"
           "--------\n"
           "# Block 7\n"
           "i24 = Goto\n"
           "# succ: 9\n"
           "--------\n"
           "# Block 8\n"
           "i30 = Phi(i14, i26)\n"
           "i32 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 9\n"
           "i26 = Literal[3]\n"
           "i28 = Goto\n"
           "# succ: 8\n"
           "--------\n"
           "# Block 10\n"
           "i40 = Return(i6)\n")

  // Phi loop
  HIR_TEST("i = 10\n"
           "k = 0\n"
           "while (--i) {\n"
           "  j = 10\n"
           "  while (--j) {\n"
           "    k = k + 1\n"
           "  }\n"
           "}\n"
           "return k",
           "# Block 0\n"
           "i0 = Entry[0]\n"
           "i2 = Literal[10]\n"
           "i4 = Literal[0]\n"
           "i6 = Nil\n"
           "i8 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 1 (loop)\n"
           "i10 = Phi(i2, i22)\n"
           "i12 = Phi(i4, i32)\n"
           "i14 = Phi(i6, i42)\n"
           "i16 = Goto\n"
           "# succ: 2\n"
           "--------\n"
           "# Block 2\n"
           "i20 = Literal[1]\n"
           "i22 = BinOp(i10, i20)\n"
           "i24 = If(i22)\n"
           "# succ: 3 5\n"
           "--------\n"
           "# Block 3\n"
           "i26 = Literal[10]\n"
           "i28 = Goto\n"
           "# succ: 6\n"
           "--------\n"
           "# Block 4\n"
           "i60 = Goto\n"
           "# succ: 1\n"
           "--------\n"
           "# Block 5\n"
           "i62 = Goto\n"
           "# succ: 12\n"
           "--------\n"
           "# Block 6 (loop)\n"
           "i32 = Phi(i12, i50)\n"
           "i34 = Phi(i26, i42)\n"
           "i36 = Goto\n"
           "# succ: 7\n"
           "--------\n"
           "# Block 7\n"
           "i40 = Literal[1]\n"
           "i42 = BinOp(i34, i40)\n"
           "i44 = If(i42)\n"
           "# succ: 8 10\n"
           "--------\n"
           "# Block 8\n"
           "i48 = Literal[1]\n"
           "i50 = BinOp(i32, i48)\n"
           "i52 = Goto\n"
           "# succ: 9\n"
           "--------\n"
           "# Block 9\n"
           "i54 = Goto\n"
           "# succ: 6\n"
           "--------\n"
           "# Block 10\n"
           "i56 = Goto\n"
           "# succ: 11\n"
           "--------\n"
           "# Block 11\n"
           "i58 = Goto\n"
           "# succ: 4\n"
           "--------\n"
           "# Block 12\n"
           "i66 = Return(i12)\n")
TEST_END(hir)
