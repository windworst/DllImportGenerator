#include "load_function.h"

int IS_STAR(int c) { return ( '*' == (c) ); }
int IS_SPACE(int c) { return ( ' ' == (c) || '\t' == (c) || '\v' == (c) || '\r' == (c) || '\n' == (c) ); }
int IS_NUMBER (int c) { return ( '0' <= (c) && (c) <= '9' ); }
int IS_IDENTIFIER_FIRST (int c) { return ( ('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z') || ('_' == (c)) ); }
int IS_IDENTIFIER_FOLLOW (int c) { return ( IS_IDENTIFIER_FIRST(c) || IS_NUMBER(c) ); }
int IS_RETURN_VALUE_FIRST (int c) { return ( IS_IDENTIFIER_FIRST(c) || IS_STAR(c) ); }
int IS_RETURN_VALUE_FOLLOW (int c) { return ( IS_IDENTIFIER_FOLLOW(c) || IS_STAR(c) ); }

int jump_space(const char* str) {
  int start = 0;
  while(IS_SPACE(str[start]) ) ++ start;
  return start;
}

int peek_return_value(const char* str) {
  int start = 0;
  if( IS_RETURN_VALUE_FIRST(str[start]) ) {
    while(IS_RETURN_VALUE_FOLLOW(str[++start]));
    return start;
  }
  return -1;
}

int peek_bracket(const char* str) {
  int start = 0;
  if(str[start] == '(') {
    char c;
    while(c = str[++ start], '\0' != c && ')' != c);
    if(str[start] == ')') return start + 1;
  }
  return -1;
}

int peek_function(const char* str, FunctionResult * result) {
  int index = 0;
  int return_type_start = -1, return_type_len = -1, function_name_start = -1, function_name_len = -1;

  for(;;) {
    int last_index = index;
    index += jump_space(str + index);

    if (return_type_start < 0) { return_type_start = index; }

    int token_len = peek_return_value(str + index);
    if (token_len < 0) { break; }

    return_type_len = last_index - return_type_start;

    function_name_start = index;
    function_name_len = token_len;

    index += token_len;
  }

  if(return_type_len <= 0) {
    return -1;
  }

  if(function_name_len <= 0) {
    return -2;
  }

  index += jump_space(str + index);

  // load params
  int params_start = index, params_len = peek_bracket(str + params_start);

  if(params_len <= 0) {
    return -3;
  }
  index = params_start + params_len;
  index += jump_space(str + index);
  if(';' == str[index]) { ++ index; }

  result->functionStr = str;
  result->start = return_type_start;
  result->end = index;
  result->returnTypeIndex = return_type_start;
  result->returnTypeLen = return_type_len;
  result->functionIndex = function_name_start;
  result->functionLen = function_name_len;
  result->argsIndex = params_start;
  result->argsLen = params_len;

  return index;
}
