#include <stdio.h>
#include <stdlib.h>
#include "load_function.h"

void importer_output(FILE* f, const char* moduleName, FunctionResult results[], int functionCount) {
  const char * file_head          = "#pragma once\n\n"
                                    "#include <stdio.h>\n"
                                    "#include <Windows.h>\n\n"
                                    "#ifdef DLL_IMPORT_DEBUG\n"
                                    "#define _DEBUG_OUTPUT 1\n"
                                    "#else\n"
                                    "#define _DEBUG_OUTPUT 0\n"
                                    "#endif\n\n"
                                    "#define _FUNC_IMPORT(CTX, FUNC_NAME)  (((*(FARPROC*)&((CTX)->FUNC_NAME)) = GetProcAddress(((CTX)->_hm), #FUNC_NAME)) , _DEBUG_OUTPUT && printf(\"FUNC: %s => %p\\n\", #FUNC_NAME, (CTX)->FUNC_NAME) , (CTX)->FUNC_NAME != NULL)\n\n"
                                    "#define _MODULE_UNLOAD(CTX) ((NULL != (CTX) && NULL != (CTX)->_hm) && (FreeLibrary((CTX)->_hm), memset((CTX), 0, sizeof(*CTX))))\n\n";
  const char* module_load_start   = "#define _MODULE_LOAD(DLL, CTX) ((NULL != (CTX)) \\\n"
                                    "    && (((CTX)->_hm = LoadLibraryA(DLL)), _DEBUG_OUTPUT && printf(\"MODULE: %s => %p\\n\", DLL, (CTX)->_hm), (CTX)->_hm != NULL) \\\n";
  const char* func_import         = "    && _FUNC_IMPORT((CTX), %.*s) \\\n";
  const char* module_load_end     = ")\n\n";

  const char* ctx_start           = "typedef struct {\n"
                                    "  HMODULE _hm;\n";
  const char* ctx_func            = "  %-10.*s (__stdcall* %.*s)\n             %.*s;\n\n";
  const char* ctx_end             = "} _CTX;\n\n";

  const char* file_end            = "typedef _CTX %s;\n"
                                    "#define %s_load(DLL, CTX)   (_MODULE_LOAD((DLL), (CTX)))\n"
                                    "#define %s_unload(CTX)      (_MODULE_UNLOAD(CTX))\n";

  fprintf(f, "%s", file_head);
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
