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

#include "selfcheck.h"
#include <stdlib.h>
#include <string.h>

#include "multiple_err.h"

#include "multiply_lexer.h"

#include "mbf_lexer.h"

/* Status definitions for lexical analysis */
enum {
    LEX_STATUS_INIT = 0,

    LEX_STATUS_COMMENT, /* #.*?<EOL> */
    LEX_STATUS_EOL, /* EOL of Windows? Mac? */

    LEX_STATUS_BACK_FINISH, /* Finished, and break */
    LEX_STATUS_FINISH, /* Finished */
    LEX_STATUS_ERROR, /* Error */
};

#define JMP(status,dst) do{(status)=(dst);}while(0);
#define FIN(x) do{(x)=LEX_STATUS_FINISH;}while(0);
#define BFIN(x) do{(x)=LEX_STATUS_BACK_FINISH;}while(0);
#define UND(x) do{(x)=LEX_STATUS_ERROR;}while(0);
#define KEEP() do{}while(0);

/* Get one token from the char stream */
static int eat_token(struct multiple_error *err, \
        struct token *new_token, \
        const char *p, const char *endp, \
        uint32_t *pos_col, uint32_t *pos_ln, \
        const int eol_type, size_t *move_on)
{
    const char *p_init = p;
    int status = LEX_STATUS_INIT;
    /*int remain_len = endp - p;*/
    int ch = 0;
    size_t prefix_strip = 0, postfix_strip = 0;

    int is_eol = 0; /* For updating EOL and Ln */

    /* Clean template */
    new_token->value = TOKEN_UNDEFINED;
    new_token->str = (char *)p_init;
    new_token->len = 0;
    new_token->pos_col = *pos_col;
    new_token->pos_ln = *pos_ln;
    while (p != endp)
    {
        ch = *p;
        switch (status)
        {
            case LEX_STATUS_EOL:
                if (ch == CHAR_LF) { FIN(status); } else { BFIN(status); }
                break;
            case LEX_STATUS_COMMENT:
                if (IS_EOL(ch)) 
                {
                    /* Reset location */
                    *pos_col = 1;
                    *pos_ln += 1;
                    is_eol = 1;
                    /* "" (Null String) */
                    new_token->value = TOKEN_WHITESPACE;
                    FIN(status);
                }
                else
                {
                    KEEP();
                }
                break;
            case LEX_STATUS_INIT:
                if (IS_EOL(ch)) 
                {
                    /* Reset location */
                    *pos_col = 1;
                    *pos_ln += 1;
                    is_eol = 1;

                    new_token->value = TOKEN_WHITESPACE; 
                    switch (eol_type)
                    {
                        case EOL_UNIX:
                        case EOL_MAC:
                            FIN(status);
                            break;
                        case EOL_DOS:
                            JMP(status, LEX_STATUS_EOL);
                            break;
                    }
                }
                if (ch == '#')
                {
                    JMP(status, LEX_STATUS_COMMENT);
                }
                else if (IS_WHITESPACE(ch)) 
                {
                    new_token->value = TOKEN_WHITESPACE; FIN(status);
                }
                else if (ch == '>')
                {new_token->value = TOKEN_INC_PTR; FIN(status);} 
                else if (ch == '<')
                {new_token->value = TOKEN_DEC_PTR; FIN(status);} 
                else if (ch == '+')
                {new_token->value = TOKEN_INC_AT_PTR; FIN(status);} 
                else if (ch == '-')
                {new_token->value = TOKEN_DEC_AT_PTR; FIN(status);} 
                else if (ch == '.')
                {new_token->value = TOKEN_OUTPUT; FIN(status);} 
                else if (ch == ',')
                {new_token->value = TOKEN_INPUT; FIN(status);} 
                else if (ch == '[')
                {new_token->value = TOKEN_WHILE_NZ; FIN(status);} 
                else if (ch == ']')
                {new_token->value = TOKEN_LOOP; FIN(status);} 
                else if (ch == '$')
                {new_token->value = TOKEN_DEBUG; FIN(status);} 
                else {new_token->value = TOKEN_UNDEFINED; UND(status);} /* Undefined! */
                break;
            case LEX_STATUS_ERROR:
                new_token->str = NULL;
                new_token->len = 0;
                multiple_error_update(err, -MULTIPLE_ERR_LEXICAL, \
                        "%d:%d: undefined token", \
                        *pos_ln, *pos_col);
                return -MULTIPLE_ERR_LEXICAL;
                break;
            case LEX_STATUS_BACK_FINISH:
                p--;
            case LEX_STATUS_FINISH:
                goto done;
                break;
            default:
                new_token->str = NULL;
                new_token->len = 0;
                multiple_error_update(err, -MULTIPLE_ERR_LEXICAL, \
                        "%d:%d: undefined lexical analysis state, " \
                        "something impossible happened", \
                        *pos_ln, *pos_col);
                return -MULTIPLE_ERR_LEXICAL;
                break;
        }
        if (status == LEX_STATUS_BACK_FINISH) break;
        p += 1;
    }
done:
    if (!is_eol)
    {
        *pos_col += (uint32_t)(p - p_init);
    }
    if (new_token->value == TOKEN_UNDEFINED)
    {
        new_token->len = 0;
        *move_on = new_token->len;
    }
    else
    {
        new_token->len = (size_t)(p - p_init);
        *move_on = new_token->len;
        new_token->str += prefix_strip;
        new_token->len -= (size_t)(prefix_strip + postfix_strip);
    }
    return 0;
}

int mbf_tokenize(struct multiple_error *err, struct token_list **list_out, const char *data, const size_t data_len)
{
    int ret = 0;
    uint32_t pos_col = 1, pos_ln = 1;
    struct token_list *new_list = NULL;
    struct token *token_template = NULL;
    const char *data_p = data, *data_endp = data_p + data_len;

    int eol_type = eol_detect(err, data, data_len);
    size_t move_on;

    if (eol_type < 0)
    {
        goto fail;
    }

    *list_out = NULL;

    if ((new_list = token_list_new()) == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    if ((token_template = token_new()) == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    while (data_p != data_endp)
    {
        if ((ret = eat_token(err, token_template, data_p, data_endp, &pos_col, &pos_ln, eol_type, &move_on)) != 0)
        {
            goto fail;
        }
        if (token_template->value != TOKEN_WHITESPACE)
        {
            if ((ret = token_list_append_token_with_template(new_list, token_template)) != 0)
            {
                goto fail;
            }
        }
        /* Move on */
        data_p += move_on;
    }
    ret = token_list_append_token_with_configure(new_list, TOKEN_FINISH, NULL, 0, pos_col, pos_ln);
    if (ret != 0) goto fail;

    *list_out = new_list;
    ret = 0;
fail:
    if (token_template != NULL)
    {
        token_template->str = NULL;
        free(token_template);
    }
    if (ret != 0)
    {
        if (new_list != NULL) token_list_destroy(new_list);
    }
    return ret;
}

struct token_value_name_tbl_item
{
    const int value;
    const char *name;
};

static struct token_value_name_tbl_item token_value_name_tbl_items[] = 
{
    { TOKEN_INC_PTR, "++ptr" },
    { TOKEN_DEC_PTR, "--ptr" },
    { TOKEN_INC_AT_PTR, "++*ptr" },
    { TOKEN_DEC_AT_PTR, "--*ptr" },
    { TOKEN_OUTPUT, "putstr(*ptr);" },
    { TOKEN_INPUT, "*ptr=getchar();" },
    { TOKEN_WHILE_NZ, "while(*ptr) {" },
    { TOKEN_LOOP, "}" },
    { TOKEN_DEBUG, "debug" },
};
#define TOKEN_VALUE_NAME_TBL_ITEMS_COUNT (sizeof(token_value_name_tbl_items)/sizeof(struct token_value_name_tbl_item))

/* Get token name */
int mbf_token_name(char **token_name, size_t *token_name_len, const int value)
{
    size_t i;

    if (generic_token_name(token_name, token_name_len, value) == 0)
    { return 0; }

    for (i = 0; i != TOKEN_VALUE_NAME_TBL_ITEMS_COUNT; i++)
    {
        if (value == token_value_name_tbl_items[i].value)
        {
            *token_name = (char *)token_value_name_tbl_items[i].name;
            *token_name_len = strlen(token_value_name_tbl_items[i].name);
            return 0;
        }
    }
    return -1;
}

