#include "window.h"

extern "C" u32 AsciiToScancode(char val) {
	if (val >= 'a' && val <= 'z') {
		return KeyboardCodeA + (val = 'a');
	}
	if (val >= 'A' && val <= 'Z') {
		return KeyboardCodeA + (val = 'A');
	}
	if (val >= '0' && val <= '9') {
		return KeyboardCode0 + (val = '0');
	}

	switch (val) {
	case '\t': return KeyboardCodeTab;
	case '\\': return KeyboardCodeBackslash;
	case '\'': return KeyboardCodeQoute;
	case '`': return KeyboardCodeTick;
	case '~': return KeyboardCodeTilde;
	case '!': return KeyboardCode1;
	case '@': return KeyboardCode2;
	case '#': return KeyboardCode3;
	case '$': return KeyboardCode4;
	case '%': return KeyboardCode5;
	case '^': return KeyboardCode6;
	case '&': return KeyboardCode7;
	case '*': return KeyboardCode8;
	case '(': return KeyboardCode9;
	case ')': return KeyboardCode0;
	case '_': return KeyboardCodeUnderscore;
	case '+': return KeyboardCodePlus;
	case '-': return KeyboardCodeMinus;
	case '=': return KeyboardCodeEquals;
	case '[': return KeyboardCodeLBracket;
	case '{': return KeyboardCodeLBrace;
	case ']': return KeyboardCodeRbracket;
	case '}': return KeyboardCodeRBrace;
	case '|': return KeyboardCodeCarray;
	case ';': return KeyboardCodeSemicolon;
	case ':': return KeyboardCodeColon;
	case '"': return KeyboardCodeQoute;
	case ',': return KeyboardCodeComma;
	case '<': return KeyboardCodeLess;
	case '.': return KeyboardCodePeriod;
	case '>': return KeyboardCodeGreater;
	case '/': return KeyboardCodeSlash;
	case '?': return KeyboardCodeQuestionmark;
	default: return 0;
	}

	return 0;
}

extern "C" char ScanCodeToAscii(u32 scanCode, bool shift) {
	if (scanCode >= KeyboardCodeA && scanCode <= KeyboardCodeZ) {
		if (shift) {
			return 'A' + (scanCode - KeyboardCodeA);
		}
		else {
			return 'a' + (scanCode - KeyboardCodeA);
		}
	}
	else if (scanCode == KeyboardCodeDelete) {
		return '\a'; // Hacky at best :(
	}
	else if (scanCode == KeyboardCodeBackspace) { //    3
		return '\b';
	}
	else if (scanCode == KeyboardCodeReturn) { //    5
		return '\n';
	}
	else if (scanCode == KeyboardCodeSpace) { //   11
		return ' ';
	}
	else if (scanCode == KeyboardCode0) { //   17
		if (shift) {
			return ')';
		}
		else {
			return '0';
		}
	}
	else if (scanCode == KeyboardCode1) { //   18
		if (shift) {
			return '!';
		}
		else {
			return '1';
		}
	}
	else if (scanCode == KeyboardCode2) { //   19
		if (shift) {
			return '@';
		}
		else {
			return '2';
		}
	}
	else if (scanCode == KeyboardCode3) { //   20
		if (shift) {
			return '#';
		}
		else {
			return '3';
		}
	}
	else if (scanCode == KeyboardCode4) { //   21
		if (shift) {
			return '$';
		}
		else {
			return '4';
		}
	}
	else if (scanCode == KeyboardCode5) { //   22
		if (shift) {
			return '%';
		}
		else {
			return '5';
		}
	}
	else if (scanCode == KeyboardCode6) { //   23
		if (shift) {
			return '^';
		}
		else {
			return '6';
		}
	}
	else if (scanCode == KeyboardCode7) { //   24
		if (shift) {
			return '&';
		}
		else {
			return '7';
		}
	}
	else if (scanCode == KeyboardCode8) { //   25
		if (shift) {
			return '*';
		}
		else {
			return '8';
		}
	}
	else if (scanCode == KeyboardCode9) { //   26
		if (shift) {
			return '(';
		}
		else {
			return '9';
		}
	}
	else if (scanCode == KeyboardCodeColon) { //   53
		if (shift) {
			return ':';
		}
		else {
			return ';';
		}
	}
	else if (scanCode == KeyboardCodeEquals) { //   54
		if (shift) {
			return '+';
		}
		else {
			return '=';
		}
	}
	else if (scanCode == KeyboardCodeLess) { //   55
		if (shift) {
			return '<';
		}
		else {
			return ',';
		}
	}
	else if (scanCode == KeyboardCodeUnderscore) { //   56
		if (shift) {
			return '_';
		}
		else {
			return '-';
		}
	}
	else if (scanCode == KeyboardCodeGreater) { //   57
		if (shift) {
			return '>';
		}
		else {
			return '.';
		}
	}
	else if (scanCode == KeyboardCodeQuestionmark) { //   58
		if (shift) {
			return '?';
		}
		else {
			return '/';
		}
	}
	else if (scanCode == KeyboardCodeTick) { //   59
		if (shift) {
			return '~';
		}
		else {
			return '`';
		}
	}
	else if (scanCode == KeyboardCodeLBrace) { //   60
		if (shift) {
			return '{';
		}
		else {
			return '[';
		}
	}
	else if (scanCode == KeyboardCodeCarray) { //   61
		if (shift) {
			return '|';
		}
		else {
			return '\\';
		}
	}
	else if (scanCode == KeyboardCodeRBrace) { //   62
		if (shift) {
			return '}';
		}
		else {
			return ']';
		}
	}
	else if (scanCode == KeyboardCodeQoute) { //   63
		if (shift) {
			return '"';
		}
		else {
			return '\'';
		}
	}
	else if (scanCode == KeyboardCodeTab) { //   64
		return '\t';
	}

	return 0;
}