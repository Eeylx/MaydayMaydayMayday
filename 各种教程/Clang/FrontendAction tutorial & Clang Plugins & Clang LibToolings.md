[TOC]





# 1. How to write RecursiveASTVisitor based ASTFrontendActions

> 本文档基于Clang6.0.1, 请注意对应版本
>
> 想要知道如何编译llvm和clang, 请参照[此文档](http://clang.llvm.org/get_started.html)



## Introduction

在本教程中, 您将学习如何创建一个使用`RecursiveASTVisitor `的`FrontendAction  `, 用来查找具有指定名称`CXXRecordDecl的`AST结点.



## Creating a FrontendAction

在编写Clang插件或者是基于LibTooling的独立工具时, 入口点通常是`FrontendAction`. `FrontendAction`是一个接口, 它可以将用户指定的操作作为编译的一部分来执行. 

为了让插件(或工具)在AST上运行, clang提供了一个很方便的接口`ASTFrontendAction`, 它负责执行操作. 

剩下的工作是实现`CreateASTConsumer`方法, 该方法针对每个翻译单元返回一个`ASTConsumer`.

```cpp
class FindNamedClassAction : public clang::ASTFrontendAction {
public:
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
		clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
		return std::unique_ptr<clang::ASTConsumer>(
            new FindNamedClassConsumer);
    }
};
```



## Creating an ASTConsumer

`ASTConsumer`是用于编写要在AST上进行的通用操作的接口, 无论这个AST是如何生成的.

`ASTConsumer`提供了许多不同的入口点, 但在本教程中唯一需要关心的是`HandleTranslationUnit`, (后面没太看懂, 放原文).

> but for our use case the only one needed is HandleTranslationUnit, which is called with the ASTContext for the translation unit. 

```cpp
class FindNamedClassConsumer : public clang::ASTConsumer {
public:
	virtual void HandleTranslationUnit(clang::ASTContext &Context) {
		// Traversing the translation unit decl via a RecursiveASTVisitor
		// will visit all nodes in the AST.
		Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
private:
	// A RecursiveASTVisitor implementation.
	FindNamedClassVisitor Visitor;
};
```



## Using the RecursiveASTVisitor

现在一切都ok了, 下一步就是实现`RecursiveASTVisitor`用来从AST中提取相关信息.

`RecursiveASTVisitor`为大多数AST节点提供了形式为`bool VisitNodeType(NodeType *)`的钩子, `TypeLoc`节点除外, 因为它是按值传递的. 我们只需要实现相关节点类型的方法.

首先编写一个访问所有`CXXRecordDecl`的`RecursiveASTVisitor`.

```cpp
class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
	bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
		// For debugging, dumping the AST nodes will show which nodes are already
		// being visited.
		Declaration->dump();

		// The return value indicates whether we want the visitation to proceed.
		// Return false to stop the traversal of the AST.
		return true;
    }
};
```

在我们的`RecursiveASTVisitor`方法中, 我们可以使用Clang AST的全部功能来研究我们感兴趣的部分. 例如, 要查找具有特定名称的所有类型声明, 我们可以检查满足条件的名称.

```cpp
bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
	if (Declaration->getQualifiedNameAsString() == "n::m::C")
		Declaration->dump();
	return true;
}
```



## Accessing the SourceManager and ASTContext

一些有关AST的信息, 如源位置和全局标识符信息, 不会存储在AST节点本身中, 而是存储在`ASTContext`及其相关的`SourceManager`中. 为了检索他们, 我们需要将`ASTContext`交给我们的`RecursiveASTVisitor`来处理.

在调用`CreateASTConsumer`期间可以从`CompilerInstance`中得到`ASTContext`. 因此我们可以将它提取出来并交给我们新创建的`FindNamedClassConsumer`.

```cpp
virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
	return std::unique_ptr<clang::ASTConsumer>(
		new FindNamedClassConsumer(&Compiler.getASTContext())
    );
}
```

现在`ASTContext`已经在`RecursiveASTVisitor`中可用了, 我们可以使用AST结点做更多有趣的事情, 比如查找他们在源中的位置.

```cpp
bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString() == "n::m::C") {
		// getFullLoc uses the ASTContext's SourceManager to resolve the source
		// location and break it up into its line and column parts.
		FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getLocStart());
		if (FullLocation.isValid())
			llvm::outs() << "Found declaration at "
			<< FullLocation.getSpellingLineNumber() << ":"
			<< FullLocation.getSpellingColumnNumber() << "\n";
    }
	return true;
}
```



## Putting it all together

现在我们将上述所有内容组合成一个例子程序.

```cpp
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

class FindNamedClassVisitor 
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
	explicit FindNamedClassVisitor(ASTContext *Context) 
        : Context(Context) {}

	bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
		if (Declaration->getQualifiedNameAsString() == "n::m::C") {
		FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getLocStart());
		if (FullLocation.isValid())
			llvm::outs() << "Found declaration at "
			<< FullLocation.getSpellingLineNumber() << ":"
			<< FullLocation.getSpellingColumnNumber() << "\n";
        }
        return true;
    }

private:
	ASTContext *Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
	explicit FindNamedClassConsumer(ASTContext *Context)
        : Visitor(Context) {}

	virtual void HandleTranslationUnit(clang::ASTContext &Context) {
		Visitor.TraverseDecl(Context.getTranslationUnitDecl());
	}
private:
	FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
		clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
		return std::unique_ptr<clang::ASTConsumer>(
			new FindNamedClassConsumer(&Compiler.getASTContext()));
    }
};

int main(int argc, char **argv) {
	if (argc > 1) {
		clang::tooling::runToolOnCode(new FindNamedClassAction, argv[1]);
	}
}
```

我们将他存储到名为`FindClassDecls.cpp`的文件中, 并创建以下的`CMkaeLists.txt`来链接它.

```cmake
set(LLVM_LINK_COMPONENTS support)
add_clang_executable(find-class-decls FindClassDecls.cpp)
target_link_libraries(find-class-decls 
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )
```



## 构建并使用工具

1. 在`llvm_source/tools/clang/tools/extra/`目录下新建工具文件夹`find-class-decls`, 将上面的两个文件`FindClassDecls.cpp`和`CMakeLists.txt`放到该目录中 
2. 在工具文件夹上层目录`extra`中的`CMakeLists.txt`文件中添加一行信息

```bash
add_subdirectory(find-class-decls)
```

3. 在`llvm_build`目录下执行`make clang`命令
4. 在`llvm_build`目录下执行`make find-class-decls`命令编译工具
5. 生成的可执行文件在`./llvm_build/bin/`目录下

> 其中工具目录下的CMakeLists.txt和extra目录下的CMakeLists.txt中的对应信息必须与工具目录的目录名相同, 在本例中对应的是`find-class-decls`. 在`bin`目录下生成的可执行文件也叫这个名字.



当通过一个小的代码片段运行这个工具时, 它会输出它所找到的所有类`n::m::C`的声明.

```bash
$ ./bin/find-class-decls "namespace n { namespace m { class C {}; } }"
Found declaration at 1:29
```



## 总结

1. 入口点是`FindNamedClassAction`, 需要实现其中的`CreateASTConsumer`方法

   其中new了一个`FindNamedClassConsumer`对象

2. 在`FindNamedClassConsumer`中实现具体的操作, 需要实现其中的`HandleTranslationUnit`方法

   其中声明了一个`FindNamedClassVisitor`类型的Visitor变量

3. 在`FindNamedClassVisitor`中实现`VisitCXXRecordDecl`方法, 用来从AST中提取信息

   在实现`CreateASTConsumer`方法时可以从`CompilerInstance`中得到`ASTContext`, 将其传给`FindNamedClassConsumer`能够在遍历AST的时候获取更多的信息.

4. 编写完整的程序, 将其链接到clang上, 并运行



## 附录

[官网链接](http://releases.llvm.org/6.0.1/tools/clang/docs/RAVFrontendAction.html)







# 2. Clang Plugins

Clang插件可以在编译期间运行额外的用户定义的操作. 本文将提供如何编写和运行Clang插件的基本教程.

> 本文档基于Clang7教程, 请注意对应版本

## Introduction

Clang插件通过代码运行`FrontendActions`. 请参阅[FrontendAction tutorial](# How to write RecursiveASTVisitor based ASTFrontendActions), 了解如何使用`RecursiveASTVisitor`. 在本教程中, 我们将演示如何编写简单的Clang插件.



## Writing a PluginASTAction

与写一个普通的`FrontendAction`的区别在于, `PluginASTAction`可以处理插件的命令行选项. `PluginASTAction`类声明了一个`ParseArgs`方法, 你必须在你的插件中实现该方法.

```cpp
bool ParseArgs(const CompilerInstance &CI,
               const std::vector<std::string>& args) {
	for (unsigned i = 0, e = args.size(); i != e; ++i) {
        if (args[i] == "-some-arg") {
            // Handle the command line argument.
        }
    }
    return true;
}
```



## Registering a plugin

编译器在运行时从动态库加载插件. 使用`FrontendPluginRegistry::Add<>`在库中注册插件.

```cpp
static FrontendPluginRegistry::Add<MyPlugin> X("my-plugin-name", "my plugin description");
```



## Defining pragmas

插件可以通过使用`PragmaHandler`定义编译指示, 并且使用`PragmaHandlerRegistry::Add<>`来注册.

```cpp
// Define a pragma handler for #pragma example_pragma
class ExamplePragmaHandler : public PragmaHandler {
public:
    ExamplePragmaHandler() : PragmaHandler("example_pragma") { }
    void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                    Token &PragmaTok) {
        // Handle the pragma
    }
};

static PragmaHandlerRegistry::Add<ExamplePragmaHandler> Y("example_pragma","example pragma description");
```



## Putting it all together

让我们看一个打印top-level函数名的示例插件. 这个例子被添加到了clang仓库中, 请查看最新版本的[PrintFunctionNames.cpp](http://llvm.org/viewvc/llvm-project/cfe/trunk/examples/PrintFunctionNames/PrintFunctionNames.cpp?view=markup).



## Running the plugin

### Using the cc1 command line

要运行插件, 必须通过命令行参数`-load`来加载包含插件注册表的动态库. 这将加载所有已经注册的插件, 并且你可以通过`-plugin`参数来选择要运行的插件. 可以使用`-plugin-arg-<plugin-name>`传递插件的其他参数.



请注意, 这些选项必须能够到达clang的cc1进程. 有两种方法可以到达:

+ 通过使用`-cc1`参数直接调用解析过程. 这种方法的缺点就是不能配置头文件的默认搜索路径, 所以需要在命令行上指定完整的系统路径.
+ 像往常一样使用clang, 但需要在所有参数之前加上参数`-Xclang`使参数被传递给cc1



例如, 要使用clang在源文件上运行插件`print-function-names`, 首先要构建插件, 然后调用带有插件的clang.

```bash
$ export BD=/path/to/build/directory
$ (cd $BD && make PrintFunctionNames )
$ clang++ -D_GNU_SOURCE -D_DEBUG -D__STDC_CONSTANT_MACROS \
      -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D_GNU_SOURCE \
      -I$BD/tools/clang/include -Itools/clang/include -I$BD/include \
      -Iinclude tools/clang/tools/clang-check/ClangCheck.cpp -fsyntax-only \
      -Xclang -load -Xclang $BD/lib/PrintFunctionNames.so -Xclang \
      -plugin -Xclang print-fns
```

另外, 请参阅print-function-name插件示例的[README](http://llvm.org/viewvc/llvm-project/cfe/trunk/examples/PrintFunctionNames/README.txt?view=markup): 

```bash
# This is a simple example demonstrating how to use clang's facility for 
# providing AST consumers using a plugin.

# Build the plugin by running `make` in this directory.

# Once the plugin is built, you can run it using:
# --
# Linux:
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c

# Mac:
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c
```



### Using the clang command line

使用clang的`-fplugin=plugin`参数将插件传递给cc1, 作为cc1中`-load`命令的参数. 如果插件实现了`getActionType`方法, 那么插件就会自动运行. 例如: 在主要的AST行为之后自动运行插件(即与使用`-add-plugin`相同).

```cpp
// Automatically run the plugin after the main AST action
PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
}
```



## 构建并使用工具

1. 进入你构建llvm和clang的目录`llvm_build`
2. 在`tools/clang/examples/PrintFunctionNames`目录下执行`make PrintFunctionNames`命令
3. 编译完成后, 执行`./bin/clang -cc1 -load ./lib/PrintFunctionNames.so -plugin print-fns test.c`





## Example : PrintFunctionNames.cpp

[该例子所在的llvm官方页面](http://llvm.org/viewvc/llvm-project/cfe/trunk/examples/PrintFunctionNames/PrintFunctionNames.cpp?view=markup)

```cpp
//===- PrintFunctionNames.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

namespace {

class PrintFunctionsConsumer : public ASTConsumer {
    CompilerInstance &Instance;
    std::set<std::string> ParsedTemplates;

public:
    PrintFunctionsConsumer(CompilerInstance &Instance,
                           std::set<std::string> ParsedTemplates)
        : Instance(Instance), ParsedTemplates(ParsedTemplates) {}

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i) {
            const Decl *D = *i;
            if (const NamedDecl *ND = dyn_cast<NamedDecl>(D))
                llvm::errs() << "top-level-decl: \"" << ND->getNameAsString() << "\"\n";
        }
        
        return true;
    }

    void HandleTranslationUnit(ASTContext& context) override {
        if (!Instance.getLangOpts().DelayedTemplateParsing)
            return;

        // This demonstrates how to force instantiation of some templates in
        // -fdelayed-template-parsing mode. (Note: Doing this unconditionally for
        // all templates is similar to not using -fdelayed-template-parsig in the
        // first place.)
        // The advantage of doing this in HandleTranslationUnit() is that all
        // codegen (when using -add-plugin) is completely finished and this can't
        // affect the compiler output.
        struct Visitor : public RecursiveASTVisitor<Visitor> {
            const std::set<std::string> &ParsedTemplates;
            Visitor(const std::set<std::string> &ParsedTemplates)
                : ParsedTemplates(ParsedTemplates) {}
            bool VisitFunctionDecl(FunctionDecl *FD) {
                if (FD->isLateTemplateParsed() &&
                    ParsedTemplates.count(FD->getNameAsString()))
                    LateParsedDecls.insert(FD);
                return true;
            }

            std::set<FunctionDecl*> LateParsedDecls;
        } v(ParsedTemplates);
        v.TraverseDecl(context.getTranslationUnitDecl());
        clang::Sema &sema = Instance.getSema();
        for (const FunctionDecl *FD : v.LateParsedDecls) {
            clang::LateParsedTemplate &LPT =
                *sema.LateParsedTemplateMap.find(FD)->second;
            sema.LateTemplateParser(sema.OpaqueParser, LPT);
            llvm::errs() << "late-parsed-decl: \"" << FD->getNameAsString() << "\"\n";
        }
    }
};

class PrintFunctionNamesAction : public PluginASTAction {
    std::set<std::string> ParsedTemplates;
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   llvm::StringRef) override {
        return llvm::make_unique<PrintFunctionsConsumer>(CI, ParsedTemplates);
    }

    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override {
        for (unsigned i = 0, e = args.size(); i != e; ++i) {
            llvm::errs() << "PrintFunctionNames arg = " << args[i] << "\n";

            // Example error handling.
            DiagnosticsEngine &D = CI.getDiagnostics();
            if (args[i] == "-an-error") {
                unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error,
                                                    "invalid argument '%0'");
                D.Report(DiagID) << args[i];
                return false;
            } else if (args[i] == "-parse-template") {
                if (i + 1 >= e) {
                    D.Report(D.getCustomDiagID(DiagnosticsEngine::Error,
                                               "missing -parse-template argument"));
                    return false;
                }
                ++i;
                ParsedTemplates.insert(args[i]);
            }
        }
        if (!args.empty() && args[0] == "help")
            PrintHelp(llvm::errs());
	
        return true;
    }
    void PrintHelp(llvm::raw_ostream& ros) {
        ros << "Help for PrintFunctionNames plugin goes here\n";
    }
};	
}

static FrontendPluginRegistry::Add<PrintFunctionNamesAction>
X("print-fns", "print function names");
```



## 附录

[官网链接](http://releases.llvm.org/6.0.1/tools/clang/docs/ClangPlugins.html)







# 3. Tutorial for building tools using LibTooling and LibASTMatchers



## Example1 : Matcher

```cpp
// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;
using namespace llvm;

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"

using namespace clang;
using namespace clang::ast_matchers;

// definie a matcher, give the matcher a name and bind the forStmt
// for(int i=0;i<n;i++) { }
StatementMatcher LoopMatcher =
    forStmt(
            hasLoopInit(declStmt(hasSingleDecl(varDecl(
                                                    hasInitializer(integerLiteral(equals(0)))
                                                      ).bind("initVarName")))),
            hasIncrement(unaryOperator(
                hasOperatorName("++"),
                hasUnaryOperand(declRefExpr(to(varDecl(
                                                        hasType(isInteger())
                                                      ).bind("incVarName")))))),
            hasCondition(binaryOperator(
                hasOperatorName("<"),
                hasLHS(ignoringParenImpCasts(declRefExpr(to(varDecl(
                                                                     hasType(isInteger())
                                                                   ).bind("condVarName"))))),
                hasRHS(expr(hasType(isInteger())))))
           ).bind("forLoop");

class LoopPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    const ForStmt *FS = Result.Nodes.getNodeAs<ForStmt>("forLoop");
    // We do not want to convert header files!
    if (!FS || !Context->getSourceManager().isWrittenInMainFile(FS->getForLoc()))
      return;
    const VarDecl *InitVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");
    const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    const VarDecl *CondVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");

    if (!areSameVariable(InitVar, IncVar) || !areSameVariable(InitVar, CondVar))
      return;
    llvm::outs() << "eey ~ Potential array-based loop discovered.\n";
  }

  static bool areSameVariable(const ValueDecl *First, const ValueDecl *Second) {
    return First && Second &&
           First->getCanonicalDecl() == Second->getCanonicalDecl();
  }
};

// Apply a custom category to all command-line options so that they are the only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

int main(int argc, const char **argv) {

  // CommonOptionsParser constructor will parse arguments and create a CompilationDatabase.
  // In case of error it will terminate the program.
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  // Use OptionsParser.getCompilations() and OptionsParser.getSourcePathList()
  // to retrieve CompilationDatabase and the list of input file paths.
  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  LoopPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(LoopMatcher, &Printer);

  // The ClangTool needs a new FrontendAction for each translation unit we run on.
  // Thus, it takes a FrontendActionFactory as parameter.  
  // To create a FrontendActionFactory from a given FrontendAction type, 
  // we call newFrontendActionFactory<clang::SyntaxOnlyAction>().
  return Tool.run(newFrontendActionFactory(&Finder).get());
}
```



## Example2 : AST visitor

```cpp
//------------------------------------------------------------------------------
// Tooling sample. Demonstrates:
//
// * How to write a simple source tool using libTooling.
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <sstream>
#include <string>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolingSampleCategory("Tool Sample");

/* 功能: 在函数前后增加注释, 并在if后增加注释,  例如:
**before:
**
** void foo(int* a, int *b) {
**   if (a[0] > 1) {
**     b[0] = 2;
**   }
** }
**-------------------------------------- 
**after:
**
** // Begin function foo returning void
** void foo(int* a, int *b) {
**   if (a[0] > 1)   // the 'if' part
**   {
**     b[0] = 2;
**   }
** }
** // End function foo
*/

																			// RecursiveASTVisitor
// 遍历源码 & 执行相应修改
// 实现 RecursiveASTVisitor , 可以通过重写相关方法来指定感兴趣的AST结点
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  // 在if语句后增加注释
  bool VisitStmt(Stmt *s) {
	// 只关心 if 语句
    if (isa<IfStmt>(s)) {
      IfStmt *IfStatement = cast<IfStmt>(s);
      Stmt *Then = IfStatement->getThen();

      TheRewriter.InsertText(Then->getLocStart(), "// the 'if' part\n", true, true);

      Stmt *Else = IfStatement->getElse();
      if (Else)
        TheRewriter.InsertText(Else->getLocStart(), "// the 'else' part\n", true, true);
    }

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *f) {
	// 只针对函数定义(拥有函数体), 忽视函数声明
    if (f->hasBody()) {
      Stmt *FuncBody = f->getBody();

      // Type name as string
      QualType QT = f->getReturnType();
      std::string TypeStr = QT.getAsString();

      // Function name
      DeclarationName DeclName = f->getNameInfo().getName();
      std::string FuncName = DeclName.getAsString();

      // Add comment before
      std::stringstream SSBefore;
      SSBefore << "// Begin function " << FuncName << " returning " << TypeStr
               << "\n";
      SourceLocation ST = f->getSourceRange().getBegin();
      TheRewriter.InsertText(ST, SSBefore.str(), true, true);

      // And after
      std::stringstream SSAfter;
      SSAfter << "\n// End function " << FuncName;
      ST = FuncBody->getLocEnd().getLocWithOffset(1);
      TheRewriter.InsertText(ST, SSAfter.str(), true, true);
    }

    return true;
  }

private:
  Rewriter &TheRewriter;
};
																						// ASTConsumer
// 实现 ASTConsumer 接口, 读取 Clang parser 生成的AST信息
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level declaration.
  bool HandleTopLevelDecl(DeclGroupRef DR) override {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
	  // 使用我们的 AST visitor 遍历声明
      Visitor.TraverseDecl(*b);
      (*b)->dump();
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};
																						// ASTFrontendAction
// Clang Libtooling 的入口点
// 对于提供给工具的每一个源文件, 创建一个新的 FrontendAction
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "** EndSourceFileAction for: " << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";

	// 将缓冲区的内容写入文件 (Now emit the rewritten buffer)
    TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    llvm::errs() << "** Creating AST consumer for: " << file << "\n";
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
																						// Main
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, ToolingSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // ClangTool::run accepts a FrontendActionFactory, which is then used to
  // create new objects implementing the FrontendAction interface. Here we use
  // the helper newFrontendActionFactory to create a default factory that will
  // return a new MyFrontendAction object every time.
  // To further customize this, we could create our own factory class.
  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
```



## 附录

[官网链接](http://releases.llvm.org/6.0.1/tools/clang/docs/LibASTMatchersTutorial.html)







# 4. 总结 - 在构建好的 LLVM+Clang 中添加 Clang LibTooling 的步骤

> 以下所有目录均为示意, 可能会使用不完整的路径或多写路径, 意在清晰地展示目录信息, 请勿完全照搬硬敲.
>
> 路径均基于 llvm 的源码路径 `llvm_source` 和 llvm 的编译路径 `llvm_build`

1. 在`llvm_source/tools/clang/tools/extra`下建立工具文件夹

   + `cd llvm_source/tools/clang/tools/extra`

   + `mkdir eey-tool-sample`

2. 在`llvm_source/tools/clang/tools/extra`目录下的`CMakeLists.txt`文件中添加工具文件夹信息

   + `echo 'add_subdirectory(eey-tool-sample)' >> clang/tools/extra/CMakeLists.txt`

3. 在工具文件夹`eey-tool-sample`下建立工具自己的`CMakeLists.txt`

   + `cd eey-tool-sample`

   + `vim CMakeLists.txt`

   + input :

     ```bash
     set(LLVM_LINK_COMPONENTS 
       support
       )
     
     add_clang_executable(eey-tool-sample
       ToolSample.cpp
       )
     target_link_libraries(eey-tool-sample
       PRIVATE
     
       clangTooling
       clangBasic
       clangASTMatchers
       )
     ```

     > 以上只是示例, 请根据实际需求增删

   + 注意, 其中的`eey-tool-sample`要与工具文件夹名对应; `ToolSample.cpp`要与实际编写代码的文件名对应

4. 编写`ToolSample.cpp`

5. 进入`llvm_build`目录, 编译工具

   + `cd llvm_build`
   + `cmake -G "Unix Makefiles" ../llvm_source`
   + `make clang`
   + `make eey-tool-sample`

6. 使用工具

   + `./llvm_build/bin/eey-tool-sample test.cpp --`

   > 其中`--`是告诉clang该插件使用自定义参数, 第二个`-`表示目前暂时不需要参数

7. 