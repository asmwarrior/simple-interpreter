#include "lex.h"

enum {
    STS_ACCEPT,
    STS_REJECT,
    STS_HUNGRY,
};

typedef uint8_t sts_t;

#define TR(st, tr) (state = (st), (STS_##tr))
#define REJECT TR(0, REJECT)

#define IS_ALPHA(c)  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define IS_ALNUM(c)  (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_WSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

#define TOKEN_DEFINE_1(token, str) \
static sts_t token(const uint8_t c) \
{ \
    static char state; \
    \
    switch (state) { \
        case 0: return c == (str)[0] ? TR(1, ACCEPT) : REJECT; \
        case 1: return REJECT; \
        default: return -1; \
    } \
}

#define TOKEN_DEFINE_2(token, str) \
static sts_t token(const uint8_t c) \
{ \
    static char state; \
    \
    switch (state) { \
        case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
        case 1: return c == (str)[1] ? TR(2, ACCEPT) : REJECT; \
        case 2: return REJECT; \
        default: return -1; \
    } \
}

#define TOKEN_DEFINE_3(token, str) \
static sts_t token(const uint8_t c) \
{ \
    static char state; \
    \
    switch (state) { \
        case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
        case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
        case 2: return c == (str)[2] ? TR(3, ACCEPT) : REJECT; \
        case 3: return REJECT; \
        default: return -1; \
    } \
}

#define TOKEN_DEFINE_4(token, str) \
static sts_t token(const uint8_t c) \
{ \
    static char state; \
    \
    switch (state) { \
        case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
        case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
        case 2: return c == (str)[2] ? TR(3, HUNGRY) : REJECT; \
        case 3: return c == (str)[3] ? TR(4, ACCEPT) : REJECT; \
        case 4: return REJECT; \
        default: return -1; \
    } \
}

#define TOKEN_DEFINE_5(token, str) \
static sts_t token(const uint8_t c) \
{ \
    static char state; \
    \
    switch (state) { \
        case 0: return c == (str)[0] ? TR(1, HUNGRY) : REJECT; \
        case 1: return c == (str)[1] ? TR(2, HUNGRY) : REJECT; \
        case 2: return c == (str)[2] ? TR(3, HUNGRY) : REJECT; \
        case 3: return c == (str)[3] ? TR(4, HUNGRY) : REJECT; \
        case 4: return c == (str)[4] ? TR(5, ACCEPT) : REJECT; \
        case 5: return REJECT; \
        default: return -1; \
    } \
}

static sts_t tk_name(const uint8_t c)
{
    static enum {
        tk_name_begin,
        tk_name_accum,
    } state;

    switch (state) {
    case tk_name_begin:
        return IS_ALPHA(c) || (c == '_') ? TR(tk_name_accum, ACCEPT) : REJECT;

    case tk_name_accum:
        return IS_ALNUM(c) || (c == '_') ? STS_ACCEPT : REJECT;
    }

    /* unreachable, but keeps the compiler happy */
    return 0;
}

static sts_t tk_nmbr(const uint8_t c)
{
    return IS_DIGIT(c) ? STS_ACCEPT : STS_REJECT;
}

static sts_t tk_wspc(const uint8_t c)
{
    static enum {
        tk_wspc_begin,
        tk_wspc_accum,
    } state;

    switch (state) {
    case tk_wspc_begin:
        return IS_WSPACE(c) ? TR(tk_wspc_accum, ACCEPT) : REJECT;

    case tk_wspc_accum:
        return IS_WSPACE(c) ? STS_ACCEPT : REJECT;
    }

    /* unreachable, but keeps the compiler happy */
    return 0;
}

static sts_t tk_lcom(const uint8_t c)
{
    static enum {
        tk_lcom_begin,
        tk_lcom_first_slash,
        tk_lcom_accum,
        tk_lcom_end
    } state;

    switch (state) {
    case tk_lcom_begin:
        return c == '/' ? TR(tk_lcom_first_slash, HUNGRY) : REJECT;

    case tk_lcom_first_slash:
        return c == '/' ? TR(tk_lcom_accum, HUNGRY) : REJECT;

    case tk_lcom_accum:
        return c == '\n' || c == '\r' ? TR(tk_lcom_end, ACCEPT) : STS_HUNGRY;

    case tk_lcom_end:
        return REJECT;
    }

    /* unreachable, but keeps the compiler happy */
    return 0;
}

static sts_t tk_bcom(const uint8_t c)
{
    static enum {
        tk_bcom_begin,
        tk_bcom_open_slash,
        tk_bcom_accum,
        tk_bcom_close_star,
        tk_bcom_end
    } state;

    switch (state) {
    case tk_bcom_begin:
        return c == '/' ? TR(tk_bcom_open_slash, HUNGRY) : REJECT;

    case tk_bcom_open_slash:
        return c == '*' ? TR(tk_bcom_accum, HUNGRY) : REJECT;

    case tk_bcom_accum:
        return c != '*' ? STS_HUNGRY : TR(tk_bcom_close_star, HUNGRY);

    case tk_bcom_close_star:
        return c == '/' ? TR(tk_bcom_end, ACCEPT) : TR(tk_bcom_accum, HUNGRY);
        
    case tk_bcom_end:
        return REJECT;
    }

    /* unreachable, but keeps the compiler happy */
    return 0;
}

TOKEN_DEFINE_1(tk_lpar, "(");
TOKEN_DEFINE_1(tk_rpar, ")");
TOKEN_DEFINE_1(tk_lbrc, "{");
TOKEN_DEFINE_1(tk_rbrc, "}");
TOKEN_DEFINE_2(tk_cond, "if");
TOKEN_DEFINE_4(tk_elif, "elif");
TOKEN_DEFINE_4(tk_else, "else");
TOKEN_DEFINE_5(tk_loop, "while");
TOKEN_DEFINE_1(tk_assn, "=");
TOKEN_DEFINE_2(tk_equl, "==");
TOKEN_DEFINE_2(tk_neql, "!=");
TOKEN_DEFINE_1(tk_lthn, "<");
TOKEN_DEFINE_1(tk_gthn, ">");
TOKEN_DEFINE_2(tk_lteq, "<=");
TOKEN_DEFINE_2(tk_gteq, ">=");
TOKEN_DEFINE_2(tk_conj, "&&");
TOKEN_DEFINE_2(tk_disj, "||");
TOKEN_DEFINE_1(tk_plus, "+");
TOKEN_DEFINE_1(tk_mins, "-");
TOKEN_DEFINE_1(tk_mult, "*");
TOKEN_DEFINE_1(tk_divd, "/");
TOKEN_DEFINE_1(tk_nega, "!");
TOKEN_DEFINE_4(tk_read, "read");
TOKEN_DEFINE_5(tk_prnt, "print");
TOKEN_DEFINE_1(tk_scol, ";");
TOKEN_DEFINE_1(tk_ques, "?");
TOKEN_DEFINE_1(tk_coln, ":");

static sts_t (*const tokens[TK_COUNT])(const uint8_t) = {
    tk_name,
    tk_nmbr,
    tk_wspc,
    tk_lcom,
    tk_bcom,
    tk_lpar,
    tk_rpar,
    tk_lbrc,
    tk_rbrc,
    tk_cond,
    tk_elif,
    tk_else,
    tk_loop,
    tk_assn,
    tk_equl,
    tk_neql,
    tk_lthn,
    tk_gthn,
    tk_lteq,
    tk_gteq,
    tk_conj,
    tk_disj,
    tk_plus,
    tk_mins,
    tk_mult,
    tk_divd,
    tk_nega,
    tk_read,
    tk_prnt,
    tk_scol,
    tk_ques,
    tk_coln,
};

int lex(const uint8_t *input, struct token *ranges, size_t *nranges)
{
    static struct {
        sts_t prev, curr;
    } statuses[TK_COUNT] = {
        [0 ... TK_COUNT - 1] = { STS_HUNGRY, STS_REJECT }
    };

    const uint8_t *prefix_beg = input, *prefix_end = input;
    tk_t accepted_token;
    ranges[*nranges = 1, 0].tk = TK_FBEG;

    while (*prefix_end) {
        int did_accept = 0;
        
        for (tk_t token = 0; token < TK_COUNT; ++token) {
            if (statuses[token].prev != STS_REJECT) {
                statuses[token].curr = tokens[token](*prefix_end);
            }

            if (statuses[token].curr != STS_REJECT) {
                did_accept = 1;
            }
        }

        if (!did_accept) {
            accepted_token = TK_COUNT;

            for (tk_t token = 0; token < TK_COUNT; ++token) {
                if (statuses[token].prev == STS_ACCEPT) {
                    accepted_token = token;
                }

                statuses[token].prev = STS_HUNGRY;
                statuses[token].curr = STS_REJECT;
            }

            ranges[(*nranges)++] = (struct token) {
                .beg = prefix_beg, 
                .end = prefix_end, 
                .tk = accepted_token
            };

            if (accepted_token == TK_COUNT) {
                return 0;
            } else {
                prefix_beg = prefix_end;
            }
        } else {
            prefix_end++;

            for (tk_t token = 0; token < TK_COUNT; ++token) {
                statuses[token].prev = statuses[token].curr;
            }
        }
    }

    accepted_token = TK_COUNT;

    for (tk_t token = 0; token < TK_COUNT; ++token) {
        if (statuses[token].curr == STS_ACCEPT) {
            accepted_token = token;
        }
        
        statuses[token].prev = STS_HUNGRY;
        statuses[token].curr = STS_REJECT;
    }

    ranges[(*nranges)++] = (struct token) {
        .beg = prefix_beg, 
        .end = prefix_end, 
        .tk = accepted_token
    };

    if (accepted_token == TK_COUNT) {
        return 0;
    }

    ranges[(*nranges)++].tk = TK_FEND;
    return 1;
}