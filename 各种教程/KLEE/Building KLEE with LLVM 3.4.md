## [KLEE官网](http://klee.github.io/)
## [Klee v1.4.0 官方文档列表](http://klee.github.io/releases/docs/v1.4.0/docs/)
# [BUILDING KLEE(recommended) with LLVM 3.4](http://klee.github.io/releases/docs/v1.4.0/build-llvm34/)
> 本教程参考此网页 ↑↑↑  
> 上面那个网址 ~~是个假网址!~~ 有部分错误, 具体步骤还需要辅助参考[此文档](http://klee.github.io/releases/docs/v1.3.0/build-llvm34/)  
> 开始之前先去换个国内源吧, 具体方法请自行Google或参考[此网页](https://github.com/Eeylx/MaydayMaydayMayday/blob/master/%E9%85%8D%E7%BD%AE%E7%8E%AF%E5%A2%83/%E6%9B%B4%E6%8D%A2apt-get%E6%BA%90/%E6%9B%B4%E6%8D%A2apt-get%E6%BA%90.md)

### 环境:
    Ubuntu 14.04 64-bit
    gcc / g++ 4.8 or later
    cmake 3.0 or later

## 1. Install dependencies
	sudo apt-get install
  + [x] `gcc/g++ 4.8 or later`
  + [x] `curl`
  + [x] `libcap-dev`
  + [x] `git`
  + [x] `cmake`
  + [x] `libncurses5-dev`
  + [x] `python-minimal`
  + [x] `python-pip`
  + [x] `unzip`

## 2. Install LLVM 3.4

See [Getting Started with the LLVM System](http://llvm.org/docs/GettingStarted.html) for more information.  
If you want to install it manually, please refer to the official [LLVM Getting Started documentation](http://releases.llvm.org/3.4.2/docs/GettingStarted.html).

#### 1. 打开apt-get资源文件

  ```bash
   gedit /etc/apt/sources.list
  ```

#### 2. 在source.lsit中添加以下信息后保存关闭:

```bash
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.4 main
deb-src http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.4 main
```

#### 3. 添加仓库密钥

```bash
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
sudo apt-get update
```

#### 4. 安装llvm3.4

     sudo apt-get install
  + [x] `clang-3.4`
  + [x] `llvm-3.4`
  + [x] `llvm-3.4-dev`
  + [x] `llvm-3.4-tools`

#### 5. Finally, make sure llvm-config is in your path
       sudo ln -sf /usr/bin/llvm-config-3.4 /usr/bin/llvm-config


## 3. Install constraint solver(s)
KLEE supports multiple different constraint solvers. You must install at least one to build KLEE.
>下列 `STP` `Z3` `metaSMT` 三选一, 这里安装STP

1. #### [STP](https://github.com/stp/stp) Historically KLEE was built around STP so support for this solver is the most stable. For build instructions, see [here](http://klee.github.io/releases/docs/v1.4.0/build-stp/).
  + ##### STP has a few external dependencies they are listed below as an install command for Ubuntu 14.04LTS.
        sudo apt-get install
    - [x] `cmake` 安装3.0及以上版本, 可以参考下文[可能遇到的问题](##### 遇到的问题及可行的解决方法)部分
    - [x] `bison`
    - [x] `flex`
    - [x] `libboost-all-dev`
    - [x] `python`
    - [x] `perl`
    - [x] `zlib1g-dev`

  + ##### 装完上述软件包后执行以下步骤
		 git clone https://github.com/stp/minisat.git
		 cd minisat
		 mkdir build
		 cd build
		 cmake -DSTATIC_BINARIES=ON -DCMAKE_INSTALL_PREFIX=/usr/local/ ../
		 sudo make install
		 cd ../../

		 git clone https://github.com/stp/stp.git
		 cd stp
         # 下一步切换到2.1.2版本, 如果执行之后后续的install不成功, 可以切换到2.2.0版本. 相关问题: https://github.com/stp/stp/issues/153 
		 git checkout tags/2.1.2
		 mkdir build
		 cd build
         cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DENABLE_PYTHON_INTERFACE:BOOL=OFF ..
		 make
		 sudo make install
		 cd ..

  + ##### As documented on the STP website, it is essential to run the following command before using STP (and thus KLEE)
    You can make this persistent by editing the `/etc/security/limits.conf` file.

		 ulimit -s unlimited

  + ##### 可能遇到的问题及可行的解决方法
    - 在执行上述命令时可能会因为cmake版本低于3.x而导致部分错误, 可以通过以下命令安装更新版本的cmake解决  
      [参考资料1](http://blog.csdn.net/geek_tank/article/details/70175905) [参考资料2](http://cameo54321.blogspot.jp/2014/02/installing-cmake-288-or-higher-on.html)
		   apt-get install software-properties-common
		   add-apt-repository ppa:george-edison55/cmake-3.x
		   apt-get update
		   apt-get install cmake
    - Cryptominisat4 (v4.5 or above) or C++11 support not found  
      * 不安也没事, 可选功能
      * 安装Cryptominisat, [Github地址](https://github.com/msoos/cryptominisat), [选择Cryptominisat版本](https://github.com/msoos/cryptominisat/releases)
    - Performing Test HAVE_FLAG_STDLIB_LIBCPP - Failed
      * 2.1.2 版本会出现问题, 请使用2.2.0版本或更高(至少2.2.0没有问题)
      * [stp / MacOS build issues #153](https://github.com/stp/stp/issues/153)
    - Performing Test HAVE_FLAG -Wheader-guard - Failed
      * 暂未解决, 貌似不影响后续操作
      * [stp / MacOS build issues #153](https://github.com/stp/stp/issues/153)
      * [stp / Cmake error (archlinux 64-bit) #156](https://github.com/stp/stp/issues/156)
      * [stp / compilation error on Ubuntu 15.04 64-bit #190](https://github.com/stp/stp/issues/190)
    - Could NOT find VALGRIND (missing:  VALGRIND_INCLUDE_DIR VALGRIND_PROGRAM)
      * [cryptominisat / OSX Compilation issues #157](https://github.com/msoos/cryptominisat/issues/157)

2. #### [Z3](https://github.com/z3prover/z3) is a more recent addition to KLEE but is reasonably stable. You should use Z3 version ≥ 4.4. For build instructions, see [here](https://github.com/Z3Prover/z3/blob/master/README.md).

3. #### [metaSMT](https://github.com/agra-uni-bremen/metaSMT) supports various solvers, including Boolector, STP and Z3. We recommend branch v4.rc1 (git clone -b v4.rc1 ...). For build instructions, see [here](https://github.com/agra-uni-bremen/metaSMT).


## 4. (Optional) Build uclibc and the POSIX environment model
By default, KLEE works on closed programs (programs that don’t use any external code such as C library functions). However, if you want to use KLEE to run real programs you will want to enable the KLEE POSIX runtime, which is built on top of the [uClibc](https://uclibc.org/) C library.  

To build klee-uclibc run:

	git clone https://github.com/klee/klee-uclibc.git  
	cd klee-uclibc  
	./configure --make-llvm-lib  
	make -j2  
	cd .. 

To tell KLEE to use klee-uclibc and use the POSIX runtime pass `-DENABLE_POSIX_RUNTIME=ON` and `-DKLEE_UCLIBC_PATH=<KLEE_UCLIBC_SOURCE_DIR>` to CMake when configuring KLEE in step 9 where `<KLEE_UCLIBC_SOURCE_DIR>` is the absolute path to the cloned klee-uclibc git repository.


## 5. (Optional) Get Google test sources
For unit tests we use the Google test libraries. If you don’t want to run the unit tests you can skip this step but you will need to pass -DENABLE_UNIT_TESTS=OFF to CMake when configuring KLEE in step 9.

We depend on a version 1.7.0 right now so grab the sources for it.

	curl -OL https://github.com/google/googletest/archive/release-1.7.0.zip
	unzip release-1.7.0.zip

This will create a directory called googletest-release-1.7.0.


## 6. (Optional) Install lit
For testing the lit tool is used. If you LLVM from a build tree you can skip this step as the build system will try to use llvm-lit in the directory containing the LLVM binaries.

If you don’t want to run the tests you can skip this step but you will need to pass `-DENABLE_UNIT_TESTS=OFF` and `-DENABLE_SYSTEM_TESTS=OFF` to CMake when configuring KLEE in step 9.

	pip install lit

## 7. (Optional) Install tcmalloc
By default, KLEE uses malloc_info() to observe and to restrict its memory usage. Due to limitations of malloc_info(), the maximum limit is set to 2 GB. To support bigger limits, KLEE can use TCMalloc as an alternative allocator. It is thus necessary to install TCMalloc:

	sudo apt-get install 
  + [x] libtcmalloc-minimal4 
  + [x] libgoogle-perftools-dev

When configuring KLEE in step 9 pass `-DENABLE_TCMALLOC=ON` to CMake when configuring KLEE.


## 8. Get KLEE source
	git clone https://github.com/klee/klee.git

## 9. Config KLEE
KLEE must be built “out of source” so first make a binary build directory. You can create this where ever you like.

	mkdir klee_build_dir

Now cd into the build directory and run CMake to configure KLEE where <KLEE_SRC_DIRECTORY> is the path to the KLEE git repository you cloned in step 8.

	cd klee_build_dir
	cmake <CMAKE_OPTIONS> <KLEE_SRC_DIRECTORY>

`<CMAKE_OPTIONS>` are the configuration options. These are documented in README-CMake.md.

For example if KLEE was being built with STP, the POSIX runtime, klee-uclibc and testing then the command line would look something like this

	cmake \
	  -DENABLE_SOLVER_STP=ON \                       // 3. stp
	  -DENABLE_POSIX_RUNTIME=ON \                    // 4. uclibc 
	  -DENABLE_KLEE_UCLIBC=ON \                      // 4
	  -DKLEE_UCLIBC_PATH=<KLEE_UCLIBC_SOURCE_DIR> \  // 4
	  -DGTEST_SRC_DIR=<GTEST_SOURCE_DIR> \           // 5. Google test
	  -DENABLE_SYSTEM_TESTS=ON \                     // 6. lit
	  -DENABLE_UNIT_TESTS=ON \                       // 6. lit
	  -DENABLE_TCMALLOC=ON \                         // 7. tcmalloc
	  <KLEE_SRC_DIRECTORY>                           // 8. get Klee source

For copy (but only for me)

	cmake -DENABLE_SOLVER_STP=ON -DENABLE_POSIX_RUNTIME=ON -DENABLE_KLEE_UCLIBC=ON -DKLEE_UCLIBC_PATH=/home/eeyore/work/klee-uclibc -DGTEST_SRC_DIR=/home/eeyore/work/dependencies/googletest-release-1.7.0 -DENABLE_SYSTEM_TESTS=ON -DENABLE_UNIT_TESTS=ON -DENABLE_TCMALLOC=ON /home/eeyore/work/klee

Where `<KLEE_UCLIBC_SOURCE_DIR>` is the absolute path the klee-uclibc source tree, `<GTEST_SOURCE_DIR>` is the absolute path to the Google Test source tree.

有关这一步的错误及解决方法请看[官方文档](http://klee.github.io/releases/docs/v1.4.0/build-llvm34/)的对应部分


## 10. Build KLEE
From the klee_build_dir directory created in the previous step run.

	make

有关这一步的错误及解决方法请看[官方文档](http://klee.github.io/releases/docs/v1.4.0/build-llvm34/)的对应部分


## 11. (Optional) Run the main regression test suite
If KLEE was configured with system tests enabled then you can run them like this.

	make systemtests

If you want to invoke lit manually use:

	lit test/

This way you can run individual tests or subsets of the suite:

	lit test/regression

## 12. (Optional) Build and run the unit test
If KLEE was configured with unit tests enabled then you can build and run the unit tests like this.

	make unittests

## 13. You're ready to go! Check the [Tutorials](http://klee.github.io/releases/docs/v1.4.0/tutorials/) page to try KLEE


## NOTE 
For testing real applications (e.g. Coreutils), you may need to increase your system’s open file limit (ulimit -n). Something between 10000 and 999999 should work. In most cases, the hard limit will have to be increased first, so it is best to directly edit the /etc/security/limits.conf file. or :

    ulimit -s 999999

## Extra

  + clang
    - 如果 `clang` 命令不能使用, 被告知有多个clang版本存在, 可以尝试将 `clang` 命令替换为 `clang-3.4` 
    - 可添加链接指定 `clang` 默认使用clang-3.4版本

          ln -s /usr/bin/clang-3.4 /usr/bin/clang

  + klee 
    - 编译好的klee命令可能不能在任意目录内使用(或必须手动指定目录), 可添加链接使其在任意目录下均可用
    
          ln -s /path-to-your-klee-build-dir/bin/klee /usr/bin/klee
          ln -s /path-to-your-klee-build-dir/bin/ktest-tool /usr/bin/ktest-tool
          ...
      
    - 该目录下有多个可执行文件, 可以按需添加链接

[[参考资料2]: 

[ [参考资料2]: