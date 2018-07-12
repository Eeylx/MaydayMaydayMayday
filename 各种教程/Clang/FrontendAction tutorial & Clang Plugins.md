# How to write RecursiveASTVisitor based ASTFrontendActions

> 本文档基于Clang7教程, 请注意对应版本

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

我们将他存储到名为`FindClassDecls.cpp`的文件中, 并创建以下的CMkaeLists.txt来链接它.

```cmake
add_clang_executable(find-class-decls FindClassDecls.cpp)

target_link_libraries(find-class-decls clangTooling)
```

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

[官网链接](http://clang.llvm.org/docs/RAVFrontendAction.html)







# Clang Plugins

Clang插件可以在编译期间运行额外的用户定义的操作. 本文将提供如何编写和运行Clang插件的基本教程.

> 本文档基于Clang7教程, 请注意对应版本

## Introduction

Clang插件通过代码运行`FrontendActions`. 请参阅[FrontendAction tutorial](# How to write RecursiveASTVisitor based ASTFrontendActions), 了解如何使用`RecursiveASTVisitor`. 在本教程中, 我们将演示如何编写简单的Clang插件.



## Writing a `PluginASTAction`

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

另外, 请参阅print-function-name插件示例的[README](# Example : PrintFunctionNames.cpp).



### Using the clang command line

使用clang的`-fplugin=plugin`参数将插件传递给cc1, 作为cc1中`-load`命令的参数. 如果插件实现了`getActionType`方法, 那么插件就会自动运行. 例如: 在主要的AST行为之后自动运行插件(即与使用`-add-plugin`相同).

```cpp
// Automatically run the plugin after the main AST action
PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
}
```



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

[官网链接](http://clang.llvm.org/docs/ClangPlugins.html)



# Tutorial for building tools using LibTooling and LibASTMatchers



## 附录

[官网链接](http://releases.llvm.org/6.0.1/tools/clang/docs/LibASTMatchersTutorial.html)

