#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>
#include <functional>

#include <iostream>
#include "JITEngine.h"
#include "Scanner.h"
using namespace std;

//============================== lexical analysis
static const char* lexemes[] = {
	"(", ")", "{", "}", "[", "]", ",", ";",
	"if", "else", "for", "while", "continue", "break", "return",
	"!", "++", "--",
	"=",
	"&&", "||",
	"+", "-", "*", "/", "%",
	"<", "<=", ">", ">=", "==", "!=",
	"int", "string", "void",
	"true", "false",
};

static map<string, Token> setupBuildinTokens() {
	map<string, Token> tokens;
	for (int i = 0; i < ARRAY_SIZE(lexemes); ++i) tokens[lexemes[i]] = Token((TokenID)i);
	return tokens;
}

static Token* getBuildinToken(const string& lexeme) {
	static map<string, Token> s_tokens(setupBuildinTokens());
	map<string, Token>::iterator iter = s_tokens.find(lexeme);
	return iter == s_tokens.end() ? NULL : &iter->second;
}


Scanner::Scanner(const char* src) : m_src(src) {
}
//Look Ahead ?
Token Scanner::LA(int n) {
	fetchN(n);
	return m_LAList[n - 1];
}

Token Scanner::next(int n) {
	fetchN(n);
	Token token = m_LAList[0];
	m_LAList.erase(m_LAList.begin(), m_LAList.begin() + n);
	return token;
}

void Scanner::fetchN(int n) {
	while ((int)m_LAList.size() < n) {
		for (; isspace(*m_src); ++m_src);

		if (m_src[0] == 0) {
			m_LAList.push_back(Token());
		}
		else if (m_src[0] == '/' && m_src[1] == '/') {
			while (*++m_src != '\n');
		}
		else if (m_src[0] == '/' && m_src[1] == '*') {
			++m_src;
			do { ++m_src; } while (m_src[0] != '*' || m_src[1] != '/');
			m_src += 2;
		}
		else if (isdigit(m_src[0])) {
			const char* begin = m_src;
			while (isdigit(*++m_src));
			m_LAList.push_back(Token(TID_INT, begin, m_src));
		}
		else if (m_src[0] == '"') {
			const char* begin = m_src;
			while (*++m_src != '"') {
				if (*m_src == '\\') ++m_src;
			}
			m_LAList.push_back(Token(TID_STRING, begin, ++m_src));
		}
		else if (isalpha(m_src[0]) || m_src[0] == '_') {
			const char* begin = m_src;
			do { ++m_src; } while (isalpha(m_src[0]) || m_src[0] == '_' || isdigit(m_src[0]));
			string lexeme(begin, m_src);
			if (Token* token = getBuildinToken(lexeme)) m_LAList.push_back(*token);
			else m_LAList.push_back(Token(TID_ID, begin, m_src));
		}
		else {
			string lexeme(m_src, m_src + 2);
			if (Token* token = getBuildinToken(lexeme)) {
				m_LAList.push_back(*token);
				m_src += 2;
			}
			else {
				lexeme.assign(m_src, m_src + 1);
				if (Token* token = getBuildinToken(lexeme)) {
					m_LAList.push_back(*token);
					++m_src;
				}
				else {
					printf("invalid token: %s\n", lexeme.c_str());
					//++m_src;
					ASSERT(0 && "invalid token");
				}
			}
		}
	}
}
