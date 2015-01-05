/* Multiple Brain F**k Programming Language : Lexical Scanner
   Copyright(C) 2014 Cheryl Natsu

   This file is part of multiple - Multiple Paradigm Language Interpreter

   multiple is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   multiple is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
   */

#ifndef _MBF_LEXER_H_
#define _MBF_LEXER_H_

#include <stdio.h>

#include "multiply_lexer.h"

/* Token Types */
enum
{
    TOKEN_INC_PTR = CUSTOM_TOKEN_STARTPOINT,
    TOKEN_DEC_PTR,
    TOKEN_INC_AT_PTR,
    TOKEN_DEC_AT_PTR,
    TOKEN_OUTPUT,
    TOKEN_INPUT,
    TOKEN_WHILE_NZ,
    TOKEN_LOOP,
    TOKEN_DEBUG,
};

/* Get token name */
int mbf_token_name(char **token_name, size_t *token_name_len, const int value);

/* Lexical scan source code */
int mbf_tokenize(struct multiple_error *err, struct token_list **list_out, const char *data, const size_t data_len);

#endif

