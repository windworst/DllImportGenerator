typedef struct {
  const char* functionStr;
  int start, end,
      returnTypeIndex, returnTypeLen,
      functionIndex, functionLen,
      argsIndex, argsLen;
} FunctionResult;

int peek_function(const char* str, FunctionResult * result);
