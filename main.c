#include <stdio.h>
#include <stdlib.h>
#include "load_function.h"

void importer_output(FILE* f, const char* moduleName, FunctionResult results[], int functionCount) {
  const char *file_head           = "#pragma once\n\n"
                                    "#include <stdio.h>\n"
                                    "#include <string.h>\n\n";
  const char *file_include        = "#if defined BUILD_ON_WINDOWS\n"
                                    "#include <Windows.h>\n"
                                    "#define _FUNC_IMPORT_ACTION(h,n) GetProcAddress((h),(n))\n"
                                    "#define _MODULE_UNLOAD_ACTION(h) FreeLibrary((h))\n"
                                    "#define _MODULE_LOAD_ACTION(p) LoadLibraryA((p))\n"
                                    "#define _MODULE_HANDLER HMODULE\n"
                                    "#define _STDCALL_DEF_ __stdcall\n"
                                    "#define _MODULE_FUNC FARPROC\n"
                                    "#elif defined BUILD_ON_LINUX || defined BUILD_ON_MAC\n"
                                    "#include <dlfcn.h>\n"
                                    "#define _FUNC_IMPORT_ACTION(h,n) dlsym((h),(n))\n"
                                    "#define _MODULE_UNLOAD_ACTION(h) dlclose((h))\n"
                                    "#define _MODULE_LOAD_ACTION(p) dlopen((p),RTLD_NOW|RTLD_GLOBAL)\n"
                                    "#define _MODULE_HANDLER void*\n"
                                    "#define _STDCALL_DEF_\n"
                                    "#define _MODULE_FUNC void*\n"
                                    "#endif\n\n";
  const char * module_loader      = "#if defined DLL_IMPORT_DEBUG\n"
                                    "#define _LOG(...) (fprintf(stdout, __VA_ARGS__) && fflush(stdout))\n"
                                    "#else\n"
                                    "#define _LOG(...) (1)\n"
                                    "#endif\n\n"
                                    "#define _FUNC_IMPORT(CTX, FUNC_NAME)  (((*(_MODULE_FUNC*)&((CTX)->FUNC_NAME)) = _FUNC_IMPORT_ACTION(((CTX)->_hm), #FUNC_NAME)) , _LOG(\"FUNC: %s => %p\\n\", #FUNC_NAME, (CTX)->FUNC_NAME) , (CTX)->FUNC_NAME != NULL)\n\n"
                                    "#define _MODULE_UNLOAD(CTX) ((NULL != (CTX) && NULL != (CTX)->_hm) && (_MODULE_UNLOAD_ACTION((CTX)->_hm), memset((CTX), 0, sizeof(*CTX))))\n\n";
  const char* module_load_start   = "#define _MODULE_LOAD(DLL, CTX) ((NULL != (CTX)) \\\n"
                                    "    && (((CTX)->_hm = _MODULE_LOAD_ACTION(DLL)), _LOG(\"MODULE: %s => %p\\n\", DLL, (CTX)->_hm), (CTX)->_hm != NULL) \\\n";
  const char* func_import         = "    && _FUNC_IMPORT((CTX), %.*s) \\\n";
  const char* module_load_end     = ")\n\n";

  const char* ctx_start           = "typedef struct {\n"
                                    "  _MODULE_HANDLER _hm;\n";
  const char* ctx_func            = "  %.*s\n             (_STDCALL_DEF_* %.*s)\n             %.*s;\n\n";
  const char* ctx_end             = "} _CTX;\n\n";

  const char* file_end            = "typedef _CTX %s;\n"
                                    "#define %s_load(DLL, CTX)   (_MODULE_LOAD((DLL), (CTX)))\n"
                                    "#define %s_unload(CTX)      (_MODULE_UNLOAD(CTX))\n";

  fprintf(f, "%s", file_head);
  fprintf(f, "%s", file_include);
  fprintf(f, "%s", module_loader);
  fprintf(f, "%s", module_load_start);
  for(int i = 0; i < functionCount; ++ i) {
    FunctionResult* result = results + i;
    fprintf(f, func_import, result->functionLen, result->functionStr + result->functionIndex);
  }
  fprintf(f, "%s", module_load_end);
  fprintf(f, "%s", ctx_start);
  for(int i = 0; i < functionCount; ++ i) {
    FunctionResult* result = results + i;
    fprintf(f, ctx_func,
            result->returnTypeLen, result->functionStr + result->returnTypeIndex,
            result->functionLen,   result->functionStr + result->functionIndex,
            result->argsLen,       result->functionStr + result->argsIndex
        );
  }
  fprintf(f, "%s", ctx_end);
  fprintf(f, file_end, moduleName, moduleName, moduleName);
}

int main(int argc, char ** argv) {
  if(argc < 3) {
    printf("    Dll Import Generator\n");
    printf("Usage:\n");
    printf("    %s <Module Name> <Function List File>\n\n", argv[0]);
    return 0;
  }
  const char * moduleName = argv[1];
  const char * filePath = argv[2];
  FILE *fp=fopen(filePath, "r");
  if (!fp) {
    fprintf(stderr, "open file error\n");
    return -1;
  }
  fseek(fp, 0L, SEEK_END);
  int source_size = ftell(fp);
  rewind(fp);
  char* source = (char*)malloc(source_size * sizeof(char*));
  fread(source, source_size, 1, fp);
  fclose(fp);

  int maxCount = 100, count = 0;
  FunctionResult * results = (FunctionResult*) malloc(maxCount * sizeof(FunctionResult));

  for(int index = 0;;) {
    int func_len = peek_function(source + index, results + count);
    if(func_len < 0) break;
    index += func_len;
    if( ++ count >= maxCount ) {
      results = realloc(results, (maxCount <<= 1) * sizeof(FunctionResult));
    }
  }
  if(count > 0) {
    importer_output(stdout, moduleName, results, count);
  }

  free(results);
  free(source);

  return 0;
}
