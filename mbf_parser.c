/* Multiple Brain F**k Programming Language : Parser
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

#include "selfcheck.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "multiple_misc.h"
#include "multiple_err.h"

#include "mbf_lexer.h"
#include "mbf_parser.h"

#include "vm_opcode.h"

int mbf_parse(struct multiple_error *err, \
        struct token_list *tokens)
{
    int ret = 0;
    struct token *token_cur;
    int loop_balance = 0;

    /* Loop Balance Checking */
    token_cur = tokens->begin;
    while ((token_cur != NULL) && (token_cur->value != TOKEN_FINISH))
    {
        switch (token_cur->value)
        {
            case TOKEN_WHILE_NZ:
                loop_balance++;
                break;
            case TOKEN_LOOP:
                loop_balance--;
                if (loop_balance < 0)
                {
                    multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                            "%d:%d: error: matching left bracket not found", \
                            token_cur->pos_ln, token_cur->pos_col);
                    ret = -MULTIPLE_ERR_PARSING;
                    goto fail;
                }
                break;
        }
        token_cur = token_cur->next; 
    }

fail:
    return ret;
}

