/* Multiple Brain F**k Programming Language : Intermediate Code Generator
 * Copyright(C) 2014 Cheryl Natsu

 * This file is part of multiple - Multiple Paradigm Language Interpreter

 * multiple is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * multiple is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "selfcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multiple_ir.h"
#include "multiple_misc.h" 
#include "multiple_err.h"

#include "multiply.h"
#include "multiply_assembler.h"

#include "vm_predef.h"
#include "vm_opcode.h"
#include "vm_types.h"

#include "mbf_lexer.h"
#include "mbf_icg.h"

#define BUFFER_ITER_NAME_LEN 10

static int mbf_icg_add_built_in_procs_init( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_RAW , OP_LSTMK   , 1          ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "arr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "len"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,
                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_inc_ptr( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    /* Increase the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,
                    /* If extending the array is needed (ptr == len) */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP     , OP_L       ,
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,
                    /* Append */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_LSTADD  ,
                    /* Increase len */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "len"      ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,
                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_inc_ptr_n( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        int level, \
        size_t count)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0, LBL_SKIP2 = 1, LBL_HEAD = 2;
    char buffer_iter_name[BUFFER_ITER_NAME_LEN];

    sprintf(buffer_iter_name, "i%05d", level);

    if ((ret = multiply_asm(err, icode, res_id,
                    /* Initialize Counter */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , count      ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,

                    MULTIPLY_ASM_LABEL  , LBL_HEAD   ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_EQ      , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP2  ,

                    /* Increase the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,
                    /* If extending the array is needed (ptr == len) */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP     , OP_L       ,
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,
                    /* Append */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_LSTADD  ,
                    /* Increase len */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "len"      ,
                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,
                    MULTIPLY_ASM_OP_LBL , OP_JMP     , LBL_HEAD   ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_dec_ptr( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    /* If extending needed */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_G       ,
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,
                    /* Append */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_LSTADDH ,
                    /* Increase the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,
                    /* Increase the len */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "len"      ,
                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,
                    /* Decrease the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_dec_ptr_n(struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        int level, \
        size_t count)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0, LBL_SKIP2 = 1, LBL_HEAD = 2;
    char buffer_iter_name[BUFFER_ITER_NAME_LEN];

    sprintf(buffer_iter_name, "i%05d", level);

    if ((ret = multiply_asm(err, icode, res_id,
                    /* Initialize Counter */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , count      ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,

                    MULTIPLY_ASM_LABEL  , LBL_HEAD   ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_EQ      , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP2  ,

                    /* If extending needed */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_G       ,
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,
                    /* Append */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_LSTADDH ,
                    /* Increase the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,
                    /* Increase the len */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "len"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "len"      ,
                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,
                    /* Decrease the pointer */
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , "ptr"      ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,
                    MULTIPLY_ASM_OP_LBL , OP_JMP     , LBL_HEAD   ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_inc_at_ptr( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0xFF       ,
                    MULTIPLY_ASM_OP     , OP_ANDA    , 
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFSET  , 
                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_inc_at_ptr_n( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        int level, \
        size_t count)
{
    int ret = 0;
    const uint32_t LBL_SKIP2 = 1, LBL_HEAD = 2;
    char buffer_iter_name[BUFFER_ITER_NAME_LEN];

    sprintf(buffer_iter_name, "i%05d", level);

    if ((ret = multiply_asm(err, icode, res_id,
                    /* Initialize Counter */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , count      ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,

                    MULTIPLY_ASM_LABEL  , LBL_HEAD   ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_EQ      , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP2  ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0xFF       ,
                    MULTIPLY_ASM_OP     , OP_ANDA    , 
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFSET  , 

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,
                    MULTIPLY_ASM_OP_LBL , OP_JMP     , LBL_HEAD   ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_dec_at_ptr( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 

                    MULTIPLY_ASM_OP     , OP_DUP     , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_G       , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,

                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 256        ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 

                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,

                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFSET  , 

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_dec_at_ptr_n( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        int level, \
        size_t count)
{
    int ret = 0;
    const uint32_t LBL_SKIP = 0, LBL_SKIP2 = 1, LBL_HEAD = 2;
    char buffer_iter_name[BUFFER_ITER_NAME_LEN];

    sprintf(buffer_iter_name, "i%05d", level);

    if ((ret = multiply_asm(err, icode, res_id,
                    /* Initialize Counter */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , count      ,
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,

                    MULTIPLY_ASM_LABEL  , LBL_HEAD   ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_EQ      , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP2  ,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 
                    MULTIPLY_ASM_OP     , OP_DUP     , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0          ,
                    MULTIPLY_ASM_OP     , OP_G       , 
                    MULTIPLY_ASM_OP_LBL , OP_JMPC    , LBL_SKIP   ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 256        ,
                    MULTIPLY_ASM_OP     , OP_ADD     , 
                    MULTIPLY_ASM_LABEL  , LBL_SKIP   ,

                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFSET  , 


                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          ,
                    MULTIPLY_ASM_OP     , OP_SUB     , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,
                    MULTIPLY_ASM_OP_LBL , OP_JMP     , LBL_HEAD   ,

                    MULTIPLY_ASM_LABEL  , LBL_SKIP2  ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_putchar( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 
                    MULTIPLY_ASM_OP_RAW , OP_FASTLIB , OP_FASTLIB_PUTCHAR , 
                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icg_add_built_in_procs_getchar( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_RAW , OP_FASTLIB , OP_FASTLIB_GETCHAR , 
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFSET  , 
                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icodegen_generic(struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        struct token **token_cur_in_out, \
        int level, \
        int verbose);

static int mbf_icodegen_while(struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        struct token **token_cur_in_out, \
        int level, \
        int verbose)
{
    int ret = 0;
    struct token *token_cur = *token_cur_in_out;
    uint32_t instrument_number_while;
    uint32_t instrument_number_jmp;
    uint32_t instrument_number_jmpc;
    uint32_t instrument_number_tail;
    char buffer_iter_name[BUFFER_ITER_NAME_LEN];

    sprintf(buffer_iter_name, "v%05d", level);

    (void)verbose;

    instrument_number_while = (uint32_t)(icode->text_section->size);

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_REFGET  , 
                    MULTIPLY_ASM_OP_ID  , OP_POP     , buffer_iter_name,

                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , buffer_iter_name,
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0,
                    MULTIPLY_ASM_OP     , OP_EQ      , 

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 

    instrument_number_jmpc = (uint32_t)(icode->text_section->size);

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_RAW , OP_JMPC    , 0,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 

    if ((ret = mbf_icodegen_generic(err, icode, res_id, \
                    &token_cur, level + 1, verbose)) != 0)
    { goto fail; }

    instrument_number_jmp = (uint32_t)(icode->text_section->size);

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_RAW , OP_JMP     , 0,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 

    instrument_number_tail = (uint32_t)(icode->text_section->size);

    if ((ret = multiply_icodegen_text_fill_jmp(err, icode, \
                    instrument_number_jmp, instrument_number_while)) != 0)
    { goto fail; }
    if ((ret = multiply_icodegen_text_fill_jmp(err, icode, \
                    instrument_number_jmpc, instrument_number_tail)) != 0) 
    { goto fail; }

    goto done;
fail:
done:
    *token_cur_in_out = token_cur;
    return ret;
}

static int mbf_icg_add_built_in_procs_debug( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP_STR , OP_PUSH    , "memory:"  ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arr"      ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,
                    MULTIPLY_ASM_OP_STR , OP_PUSH    , "\n"       ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,

                    MULTIPLY_ASM_OP_STR , OP_PUSH    , "pointer:"  ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,
                    MULTIPLY_ASM_OP_ID  , OP_PUSH    , "ptr"      ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,
                    MULTIPLY_ASM_OP_STR , OP_PUSH    , "\n"       ,
                    MULTIPLY_ASM_OP     , OP_PRINT   ,

                    MULTIPLY_ASM_FINISH
                    )) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

static int mbf_icodegen_generic(struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        struct token **token_cur_in_out, \
        int level, \
        int verbose)
{
    int ret = 0;
    struct token *token_cur = *token_cur_in_out, *token_next;
    size_t count;
    int token_first_value;

    (void)verbose;

    while ((token_cur != NULL) && (token_cur->value != TOKEN_FINISH))
    {
        switch (token_cur->value)
        {
            case TOKEN_INC_PTR:
            case TOKEN_DEC_PTR:
            case TOKEN_INC_AT_PTR:
            case TOKEN_DEC_AT_PTR:
                token_first_value = token_cur->value;
                count = 1;
                token_next = token_cur->next;
                while ((token_next != NULL) && (token_next->value != TOKEN_FINISH))
                {
                    if (token_next->value != token_first_value) break;
                    token_cur = token_cur->next;
                    token_next = token_next->next;
                    count++;
                }
                if (count > 1)
                {
                    switch (token_first_value)
                    {
                        case TOKEN_INC_PTR:
                            mbf_icg_add_built_in_procs_inc_ptr_n(err, icode, res_id, level, count);
                            break;
                        case TOKEN_DEC_PTR:
                            mbf_icg_add_built_in_procs_dec_ptr_n(err, icode, res_id, level, count);
                            break;
                        case TOKEN_INC_AT_PTR:
                            mbf_icg_add_built_in_procs_inc_at_ptr_n(err, icode, res_id, level, count);
                            break;
                        case TOKEN_DEC_AT_PTR:
                            mbf_icg_add_built_in_procs_dec_at_ptr_n(err, icode, res_id, level, count);
                            break;
                    }
                }
                else
                {
                    switch (token_first_value)
                    {
                        case TOKEN_INC_PTR:
                            mbf_icg_add_built_in_procs_inc_ptr(err, icode, res_id);
                            break;
                        case TOKEN_DEC_PTR:
                            mbf_icg_add_built_in_procs_dec_ptr(err, icode, res_id);
                            break;
                        case TOKEN_INC_AT_PTR:
                            mbf_icg_add_built_in_procs_inc_at_ptr(err, icode, res_id);
                            break;
                        case TOKEN_DEC_AT_PTR:
                            mbf_icg_add_built_in_procs_dec_at_ptr(err, icode, res_id);
                            break;
                    }
                }
                break;
            case TOKEN_OUTPUT:
                mbf_icg_add_built_in_procs_putchar(err, icode, res_id);
                break;
            case TOKEN_INPUT:
                mbf_icg_add_built_in_procs_getchar(err, icode, res_id);
                break;
            case TOKEN_WHILE_NZ:
                token_cur = token_cur->next;
                if ((ret = mbf_icodegen_while(err, icode, res_id, &token_cur, level + 1, verbose)) != 0)
                { goto fail; }
                break;
            case TOKEN_LOOP:
                goto finish;
                break;
            case TOKEN_DEBUG:
                mbf_icg_add_built_in_procs_debug(err, icode, res_id);
                break;
            default:
                break;
        }
        token_cur = token_cur->next;
    }
finish:
fail:
    *token_cur_in_out = token_cur;
    return ret;
}

static int mbf_icg_export( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;
    uint32_t id;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;

    new_export_section_item = multiple_ir_export_section_item_new();
    if (new_export_section_item == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    new_export_section_item->args_count = 0;
    new_export_section_item->args = NULL;
    new_export_section_item->args_types = NULL;

    /* Append export section item */
    if ((ret = multiply_resource_get_id( \
                    err, \
                    icode, \
                    res_id, \
                    &id, \
                    "main", 4)) != 0)
    { goto fail; }

    new_export_section_item->name = id;
    new_export_section_item->instrument_number = (uint32_t)0;
    if ((ret = multiple_ir_export_section_append(icode->export_section, new_export_section_item)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }

    goto done;
fail:
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
done:
    return ret;
}

int mbf_irgen(struct multiple_error *err, \
        struct multiple_ir **icode_out, \
        struct token_list *tokens, \
        int verbose)
{
    int ret = 0;
    struct multiple_ir *new_icode = NULL;
    struct multiply_resource_id_pool *new_res_id = NULL;
    uint32_t id;
    struct token *token_cur = tokens->begin;

    *icode_out = NULL;

    if ((new_icode = multiple_ir_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); return -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_res_id = multiply_resource_id_pool_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); return -MULTIPLE_ERR_MALLOC; goto fail; }

    /* Initialize */
    if ((ret = mbf_icg_add_built_in_procs_init(err, new_icode, new_res_id)) != 0)
    { goto fail; }

    /* Start compilation */
    if ((ret = mbf_icodegen_generic(err, new_icode, new_res_id, &token_cur, 0, verbose)) != 0)
    { goto fail; }

    /* Return */
    if ((ret = multiply_resource_get_none(err, new_icode, new_res_id, &id)) != 0) 
    { goto fail; }
    if ((ret = multiply_icodegen_text_section_append(err, new_icode, OP_PUSH, id)) != 0) { goto fail; }
    if ((ret = multiply_icodegen_text_section_append(err, new_icode, OP_RETURN, 0)) != 0) { goto fail; }

    /* Export */
    if ((ret = mbf_icg_export(err, new_icode, new_res_id)) != 0)
    { goto fail; }

    ret = 0;
    *icode_out = new_icode;
    goto done;
fail:
    if (new_icode != NULL) 
    {
        multiple_ir_destroy(new_icode);
        new_icode = NULL;
    }
done:
    if (new_res_id != NULL)
    { multiply_resource_id_pool_destroy(new_res_id); }
    return ret;
}

