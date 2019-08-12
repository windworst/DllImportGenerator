#include "load_function.h"

int IS_SPACE(int c) { return ( ' ' == (c) || '\t' == (c) || '\v' == (c) || '\r' == (c) || '\n' == (c) ); }
int IS_NUMBER (int c) { return ( '0' <= (c) && (c) <= '9' ); }
int IS_IDENTIFIER_FIRST (int c) { return ( ('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z') || ('_' == (c)) ); }
int IS_IDENTIFIER_FOLLOW (int c) { return ( IS_IDENTIFIER_FIRST(c) || IS_NUMBER(c) ); }

int jump_space(const char* str) {
  int start = 0;
  while(IS_SPACE(str[start]) ) ++ start;
  return start;
}

int peek_identifier(const char* str) {
  int start = 0;
  if( IS_IDENTIFIER_FIRST(str[start]) ) {
    while(IS_IDENTIFIER_FOLLOW(str[++start]));
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
  int index = 0, start = 0;

  index += jump_space(str + index);
  start = index;

  //return type
  int return_type_start = index, return_type_len = peek_identifier(str + return_type_start);

  if(return_type_len < 0) {
    return -1;
  }
  index = return_type_start + return_type_len;
  index += jump_space(str + index);

  //function name
  int function_name_start = index, function_name_len = peek_identifier(str + function_name_start);

  if(function_name_len < 0) {
    return -2;
  }
  index = function_name_start + function_name_len;
  index += jump_space(str + index);

  // load params
  int params_start = index, params_len = peek_bracket(str + params_start);

  if(params_len < 0) {
    return -3;
  }
  index = params_start + params_len;
  index += jump_space(str + index);
  if(';' == str[index]) { ++ index; }

  result->functionStr = str;
  result->start = start;
  result->end = index;
  result->returnTypeIndex = return_type_start;
  result->returnTypeLen = return_type_len;
  result->functionIndex = function_name_start;
  result->functionLen = function_name_len;
  result->argsIndex = params_start;
  result->argsLen = params_len;

  return index;
}
