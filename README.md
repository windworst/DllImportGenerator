# Dll Importer Generator
可以快速生成Dll导入代码

### 编译
```
make
# 生成 可执行文件 dige
```

### 运行
```
    Dll Import Generator
Usage:
    dige <Module Name> <Function List File>
```

### 使用
新建 ```module.txt``` 里面放函数定义
```
int a();
void b(int xx);
...
```
执行命令
```
dige Module module.txt
```
生成代码
```
> dige Module module.txt
#pragma once

#include <stdio.h>
#include <string.h>

#if defined BUILD_ON_WINDOWS
#include <Windows.h>
#define _FUNC_IMPORT_ACTION(h,n) GetProcAddress((h),(n))
#define _MODULE_UNLOAD_ACTION(h) FreeLibrary((h))
#define _MODULE_LOAD_ACTION(p) LoadLibraryA((p))
#define _MODULE_HANDLER HMODULE
#define _STDCALL_DEF_ __stdcall
#elif defined BUILD_ON_LINUX || defined BUILD_ON_MAC
#include <dlfcn.h>
#define _FUNC_IMPORT_ACTION(h,n) dlsym((h),(n))
#define _MODULE_UNLOAD_ACTION(h) dlclose((h))
#define _MODULE_LOAD_ACTION(p) dlopen((p),0)
#define _MODULE_HANDLER void*
#define _STDCALL_DEF_
#endif

#if defined DLL_IMPORT_DEBUG
#define _LOG(...) (fprintf(stdout, __VA_ARGS__) && fflush(stdout))
#else
#define _LOG(...) (1)
#endif

#define _FUNC_IMPORT(CTX, FUNC_NAME)  (((*(FARPROC*)&((CTX)->FUNC_NAME)) = _FUNC_IMPORT_ACTION(((CTX)->_hm), #FUNC_NAME)) , _LOG("FUNC: %s => %p\n", #FUNC_NAME, (CTX)->FUNC_NAME) , (CTX)->FUNC_NAME != NULL)

#define _MODULE_UNLOAD(CTX) ((NULL != (CTX) && NULL != (CTX)->_hm) && (_MODULE_UNLOAD_ACTION((CTX)->_hm), memset((CTX), 0, sizeof(*CTX))))

#define _MODULE_LOAD(DLL, CTX) ((NULL != (CTX)) \
    && (((CTX)->_hm = _MODULE_LOAD_ACTION(DLL)), _LOG("MODULE: %s => %p\n", DLL, (CTX)->_hm), (CTX)->_hm != NULL) \
    && _FUNC_IMPORT((CTX), a) \
    && _FUNC_IMPORT((CTX), b) \
)

typedef struct {
  _MODULE_HANDLER _hm;
  int
             (_STDCALL_DEF_* a)
             ();

  void
             (_STDCALL_DEF_* b)
             (int xx);

} _CTX;

typedef _CTX Module;
#define Module_load(DLL, CTX)   (_MODULE_LOAD((DLL), (CTX)))
#define Module_unload(CTX)      (_MODULE_UNLOAD(CTX))
```
可以用
```
dige Module module.txt > module.gen.h
```
把输出保存下来.

在代码里使用
```
#define BUILD_ON_WINDOWS // Windows环境构建时
#define BUILD_ON_LINUS // Linux环境构建时
#define DLL_IMPORT_DEBUG // 定义该宏, 导入DLL时将输出调试信息
#include "module.gen.h"
...
Module module;
if(Module_load("module.dll", &module)) {
	module.a();
	module.b(0);
	....
}
Module_unload(&module);
```
调用Dll导出函数
