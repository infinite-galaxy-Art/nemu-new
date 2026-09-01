#include "nemu.h"
#include "monitor/monitor.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, NUM, REG, VAR, DEREF
};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"==",	EQ},					// equal
	{"!=",	NEQ},					// not equal
	{"&&",	AND},					// logical and
	{"\\|\\|", OR},					// logical or
	{"\\+", '+'},					// plus
	{"-",	'-'},					// minus
	{"\\*", '*'},					// multiply / dereference
	{"/",	'/'},					// divide
	{"!",	'!'},					// logical not
	{"\\(", '('},					// left parenthesis
	{"\\)", ')'},					// right parenthesis
	{"0x[0-9a-fA-F]+", NUM},		// hexadecimal number
	{"[0-9]+", NUM},				// decimal number
	{"\\$[a-z]+", REG},				// register name
	{"[a-zA-Z_][a-zA-Z0-9_]*", VAR}	// variable / identifier name
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* Record the recognized token (except spaces). */
				if(rules[i].token_type != NOTYPE) {
					/* Make sure the sub-string never overflows the buffer. */
					if(substr_len >= 32) substr_len = 31;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					tokens[nr_token].str[substr_len] = '\0';
					tokens[nr_token].type = rules[i].token_type;
					nr_token ++;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

/* Whether a token can be an operand (appears on the left of a binary operator). */
static bool is_operand(int type) {
	return type == NUM || type == REG || type == VAR || type == ')';
}

/* Priority of a binary operator; the smaller the number, the lower the
 * priority.  Returns -1 if `type' is not a binary operator. */
static int op_priority(int type) {
	switch(type) {
		case OR: return 1;
		case AND: return 2;
		case EQ:
		case NEQ: return 3;
		case '+':
		case '-': return 4;
		case '*':
		case '/': return 5;
		default: return -1;
	}
}

/* Check whether tokens[p..q] is surrounded by a matched pair of parentheses. */
static bool check_parentheses(int p, int q) {
	if(tokens[p].type != '(' || tokens[q].type != ')') {
		return false;
	}

	int depth = 0;
	int i;
	for(i = p; i <= q; i ++) {
		if(tokens[i].type == '(') depth ++;
		if(tokens[i].type == ')') depth --;
		if(depth == 0 && i != q) {
			/* The outermost parentheses close before the end of the
			 * expression, so the whole expression is not surrounded. */
			return false;
		}
	}
	return depth == 0;
}

/* Find the position of the dominant binary operator in tokens[p..q].
 * Returns -1 if there is no binary operator (e.g. a single token or a
 * unary expression). */
static int find_dominant(int p, int q) {
	int depth = 0;
	int dominant = -1;
	int dominant_prio = 0x7fffffff;
	int i;

	for(i = p; i <= q; i ++) {
		int t = tokens[i].type;
		if(t == '(') { depth ++; continue; }
		if(t == ')') { depth --; continue; }
		if(depth != 0) continue;

		/* A '-' at the head or right after an operator is a unary minus. */
		if(t == '-' && (i == p || !is_operand(tokens[i - 1].type))) {
			continue;
		}

		int prio = op_priority(t);
		if(prio < 0) continue;

		/* Left associative: keep the right-most operator of the lowest priority. */
		if(prio <= dominant_prio) {
			dominant_prio = prio;
			dominant = i;
		}
	}

	return dominant;
}

/* Evaluate a single token (a number or a register). */
static uint32_t token_val(int index, bool *success) {
	if(tokens[index].type == NUM) {
		return (uint32_t)strtoul(tokens[index].str, NULL, 0);
	}
	else if(tokens[index].type == REG) {
		const char *name = tokens[index].str + 1;	/* skip the leading '$' */
		if(strcmp(name, "eip") == 0) return cpu.eip;
		if(strcmp(name, "eflags") == 0) return cpu.eflags.val;
		int i;
		for(i = 0; i < 8; i ++) {
			if(strcmp(name, regsl[i]) == 0) return reg_l(i);
			if(strcmp(name, regsw[i]) == 0) return reg_w(i);
			if(strcmp(name, regsb[i]) == 0) return reg_b(i);
		}
	}
	else if(tokens[index].type == VAR) {
		/* A global variable: return its address from the symbol table. */
		swaddr_t addr;
		if(get_symbol_addr(tokens[index].str, &addr)) {
			return addr;
		}
	}

	*success = false;
	return 0;
}

static uint32_t eval(int p, int q, bool *success) {
	if(p > q) {
		/* Bad expression. */
		*success = false;
		return 0;
	}
	else if(p == q) {
		/* A single token: it should be a number or a register. */
		return token_val(p, success);
	}
	else if(check_parentheses(p, q)) {
		/* Surrounded by a matched pair of parentheses: throw them away. */
		return eval(p + 1, q - 1, success);
	}
	else {
		int op = find_dominant(p, q);
		if(op < 0) {
			/* No binary operator: it must be a unary expression. */
			int t = tokens[p].type;
			if(t == '-') {			/* unary minus */
				return -(int32_t)eval(p + 1, q, success);
			}
			else if(t == '!') {		/* logical not */
				return !eval(p + 1, q, success);
			}
			else if(t == DEREF) {	/* dereference */
				uint32_t addr = eval(p + 1, q, success);
				if(!*success) return 0;
				return swaddr_read(addr, 4, SREG_DS);
			}
			*success = false;
			return 0;
		}

		int op_type = tokens[op].type;
		uint32_t val1 = eval(p, op - 1, success);
		uint32_t val2 = eval(op + 1, q, success);
		if(!*success) return 0;

		switch(op_type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/':
				if(val2 == 0) {
					*success = false;
					return 0;
				}
				return val1 / val2;
			case EQ:  return val1 == val2;
			case NEQ: return val1 != val2;
			case AND: return val1 && val2;
			case OR:  return val1 || val2;
			default: *success = false; return 0;
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* Distinguish the dereference operator '*' from multiplication:
	 * a '*' whose previous token is not an operand is a dereference. */
	int i;
	for(i = 0; i < nr_token; i ++) {
		if(tokens[i].type == '*') {
			if(i == 0 || !is_operand(tokens[i - 1].type)) {
				tokens[i].type = DEREF;
			}
		}
	}

	*success = true;
	return eval(0, nr_token - 1, success);
}
