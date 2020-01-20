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
#include <Windows.h>

#ifdef DLL_IMPORT_DEBUG
#define _LOG(...) (fprintf(stdout, __VA_ARGS__) && fflush(stdout))
#else
#define _LOG(...) (1)
#endif

#define _FUNC_IMPORT(CTX, FUNC_NAME)  (((*(FARPROC*)&((CTX)->FUNC_NAME)) = GetProcAddress(((CTX)->_hm), #FUNC_NAME)) , _LOG("FUNC: %s => %p\n", #FUNC_NAME, (CTX)->FUNC_NAME) , (CTX)->FUNC_NAME != NULL)

#define _MODULE_UNLOAD(CTX) ((NULL != (CTX) && NULL != (CTX)->_hm) && (FreeLibrary((CTX)->_hm), memset((CTX), 0, sizeof(*CTX))))

#define _MODULE_LOAD(DLL, CTX) ((NULL != (CTX)) \
    && (((CTX)->_hm = LoadLibraryA(DLL)), _LOG("MODULE: %s => %p\n", DLL, (CTX)->_hm), (CTX)->_hm != NULL) \
    && _FUNC_IMPORT((CTX), a) \
    && _FUNC_IMPORT((CTX), b) \
)

typedef struct {
  HMODULE _hm;
  int
             (__stdcall* a)
             ();

  void
             (__stdcall* b)
             (int xx);

} _CTX;

typedef _CTX Module;
#define Module_load(DLL, CTX)   (_MODULE_LOAD((DLL), (CTX)))
#define Module_unload(CTX)      (_MODULE_UNLOAD(CTX))
```
可以用
```
dige Module module.txt > module.h
```
把输出保存下来.

在代码里使用
```
#define DLL_IMPORT_DEBUG // 定义该宏, 导入DLL时将输出调试信息
#include "module.h"
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
