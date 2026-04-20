//
// This program will split sources files resulting from the macro conversion.
//
/*
_test
	ldx #6 ;	lda #7 ;	jsr enter ;
	lda #<(0) ;	sta reg1 ;
	lda #<(4) ;	sta reg3 ;
	lda #<(590) ;	sta reg2 ;	lda #>(590) ;	sta reg2+1 ;
	lda reg2 ;	sta tmp0 ;	lda reg2+1 ;	sta tmp0+1 ;

Will become:

_test
	ldx #6
	lda #7
	jsr enter
	lda #<(0)
	sta reg1
	lda #<(4)
	sta reg3
	lda #<(590)
	sta reg2
	lda #>(590)
	sta reg2+1
	lda reg2
	sta tmp0
	lda reg2+1
	sta tmp0+1 ;


*/

#include "infos.h"

#include "common.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <conio.h>
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>


// ============================================================================
// Verbosity level (read from OSDKVERBOSITY environment variable)
// ============================================================================
int g_verbosity = 2;


bool ConvertBuffer(void* &pBuffer,size_t &nSizeBuffer)
{
	std::string cDestString((const char*)pBuffer,nSizeBuffer);

	// UNIX to DOS replacement
	StringReplace(cDestString,"\n\r","\r\n");

	StringReplace(cDestString,";","\r\n");

	free(pBuffer);
	nSizeBuffer=cDestString.size();
	pBuffer=malloc(nSizeBuffer+1);
	memcpy(pBuffer,cDestString.c_str(),nSizeBuffer+1);

	return true;
}


// ============================================================================
// 6502 Peephole Optimizer
// ============================================================================

enum class TokenType
{
	Instruction,
	Label,
	Directive,
	FoldOpen,
	FoldClose,
	Empty,
	Other
};

struct Token
{
	TokenType   type;
	std::string text;           // Original trimmed text
	std::string mnemonic;       // Lowercase mnemonic (Instruction only)
	std::string operand;        // Trimmed operand (Instruction only)
	bool        eliminated;     // Marked for removal
	int         lineIndex;      // Source line for reassembly
};


static bool IsValidMnemonic(const std::string& m)
{
	static const char* mnemonics[] =
	{
		"adc","and","asl","bcc","bcs","beq","bit","bmi",
		"bne","bpl","brk","bvc","bvs","clc","cld","cli",
		"clv","cmp","cpx","cpy","dec","dex","dey","eor",
		"inc","inx","iny","jmp","jsr","lda","ldx","ldy",
		"lsr","nop","ora","pha","php","pla","plp","rol",
		"ror","rti","rts","sbc","sec","sed","sei","sta",
		"stx","sty","tax","tay","tsx","txa","txs","tya"
	};
	for (int i = 0; i < (int)(sizeof(mnemonics) / sizeof(mnemonics[0])); i++)
	{
		if (m == mnemonics[i])
			return true;
	}
	return false;
}


static std::string TrimString(const std::string& s)
{
	size_t start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";
	size_t end = s.find_last_not_of(" \t");
	return s.substr(start, end - start + 1);
}


static std::string ToLower(const std::string& s)
{
	std::string result = s;
	for (size_t i = 0; i < result.size(); i++)
		result[i] = (char)tolower((unsigned char)result[i]);
	return result;
}


static Token ParseToken(const std::string& raw, int lineIndex)
{
	Token tok;
	tok.type = TokenType::Empty;
	tok.eliminated = false;
	tok.lineIndex = lineIndex;

	std::string trimmed = TrimString(raw);
	tok.text = trimmed;

	if (trimmed.empty())
	{
		tok.type = TokenType::Empty;
		return tok;
	}

	// Fold markers
	if (trimmed == ".(")
	{
		tok.type = TokenType::FoldOpen;
		return tok;
	}
	if (trimmed == ".)")
	{
		tok.type = TokenType::FoldClose;
		return tok;
	}

	// Directives: starts with . or #
	if (trimmed[0] == '.' || trimmed[0] == '#')
	{
		tok.type = TokenType::Directive;
		return tok;
	}

	// Try to parse as instruction: first word is a 3-letter mnemonic
	// Extract the first word
	size_t wordEnd = 0;
	while (wordEnd < trimmed.size() && !isspace((unsigned char)trimmed[wordEnd]))
		wordEnd++;

	std::string firstWord = ToLower(trimmed.substr(0, wordEnd));

	if (firstWord.size() == 3 && IsValidMnemonic(firstWord))
	{
		tok.type = TokenType::Instruction;
		tok.mnemonic = firstWord;
		// Everything after the mnemonic is the operand
		std::string rest = (wordEnd < trimmed.size()) ? trimmed.substr(wordEnd) : "";
		tok.operand = TrimString(rest);
		return tok;
	}

	// Otherwise it's a label (identifier-like text at column 0 or after colon)
	tok.type = TokenType::Label;
	return tok;
}


static std::vector<Token> TokenizeLine(const std::string& line, int lineIndex)
{
	std::vector<Token> tokens;
	size_t start = 0;

	while (start <= line.size())
	{
		size_t pos = line.find(':', start);
		std::string segment;
		if (pos == std::string::npos)
		{
			segment = line.substr(start);
			start = line.size() + 1;
		}
		else
		{
			segment = line.substr(start, pos - start);
			start = pos + 1;
		}

		Token tok = ParseToken(segment, lineIndex);
		if (tok.type != TokenType::Empty)
		{
			tokens.push_back(tok);
		}
	}

	return tokens;
}


static bool IsBarrier(TokenType type)
{
	return type == TokenType::Label
		|| type == TokenType::Directive;
}


// Try to parse an operand as a numeric address.
// Supports $ and 0x prefix (hex), 0 prefix (octal), and plain decimal.
// Returns true if successfully parsed, with the value in 'result'.
static bool TryParseAddress(const std::string& operand, long& result)
{
	if (operand.empty())
		return false;

	const char* str = operand.c_str();

	// Strip ,x or ,y indexing suffix for the purpose of address parsing
	std::string clean = operand;
	size_t comma = clean.find(',');
	if (comma != std::string::npos)
		clean = TrimString(clean.substr(0, comma));
	if (clean.empty())
		return false;
	str = clean.c_str();

	char* end = nullptr;

	if (str[0] == '$')
	{
		// $hex
		result = strtol(str + 1, &end, 16);
	}
	else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		// 0xhex
		result = strtol(str + 2, &end, 16);
	}
	else if (str[0] >= '0' && str[0] <= '9')
	{
		// Decimal (or octal with leading 0, handled by strtol base 0)
		result = strtol(str, &end, 0);
	}
	else
	{
		// Symbolic name - can't resolve
		return false;
	}

	// Must have consumed the entire string
	return (end != nullptr && *end == '\0');
}


// Check if an operand targets the Oric I/O page ($300-$3FF).
// Writes to this range have hardware side effects and must not be eliminated.
static bool IsIOPageAddress(const std::string& operand)
{
	long addr = 0;
	if (!TryParseAddress(operand, addr))
		return false;
	return (addr >= 0x300 && addr <= 0x3FF);
}


static bool IsConditionalBranch(const std::string& mnemonic)
{
	return mnemonic == "bne" || mnemonic == "beq"
		|| mnemonic == "bpl" || mnemonic == "bmi"
		|| mnemonic == "bcc" || mnemonic == "bcs"
		|| mnemonic == "bvc" || mnemonic == "bvs";
}


// Estimate the assembled byte size of a 6502 instruction from its mnemonic and operand.
// Used both for byte savings reporting and for resolving relative branch targets.
// When the operand is symbolic and we can't determine ZP vs absolute, we assume ZP (2 bytes).
// The branch resolver self-validates: if the estimate is wrong, the byte count won't match
// the *+N offset and the branch will be left unresolved (safe fallback).
static int EstimateInstructionSize(const std::string& mnemonic, const std::string& operand)
{
	// No operand: implied/accumulator = 1 byte
	if (operand.empty())
		return 1;

	// Branch instructions: always 2 bytes (opcode + relative offset)
	if (IsConditionalBranch(mnemonic))
		return 2;

	// Immediate: 2 bytes
	if (operand[0] == '#')
		return 2;

	// jmp and jsr: always 3 bytes
	if (mnemonic == "jmp" || mnemonic == "jsr")
		return 3;

	// Indirect modes: (zp,x) or (zp),y = 2 bytes
	if (operand[0] == '(')
		return 2;

	// Strip indexing suffix for address analysis
	std::string base = operand;
	size_t comma = base.find(',');
	if (comma != std::string::npos)
		base = TrimString(base.substr(0, comma));

	// Try to parse as numeric address
	long addr = 0;
	if (TryParseAddress(base, addr))
		return (addr < 256) ? 2 : 3;

	// Symbolic operand: assume zero page (2 bytes) for compiler-generated code.
	// If wrong, the branch resolver's self-validation will catch the mismatch.
	return 2;
}


// Parse *+N or *-N from a branch operand.
// Returns true if found, with the absolute offset in 'offset' and direction in 'forward'.
static bool ParseRelativeOffset(const std::string& operand, int& offset, bool& forward)
{
	if (operand.size() < 3 || operand[0] != '*')
		return false;
	if (operand[1] == '+')
		forward = true;
	else if (operand[1] == '-')
		forward = false;
	else
		return false;

	char* end = nullptr;
	long val = strtol(operand.c_str() + 2, &end, 10);
	if (end == nullptr || *end != '\0' || val <= 0)
		return false;
	offset = (int)val;
	return true;
}


// Resolve *+N and *-N branch targets to synthetic labels.
// This makes the labels act as natural optimization barriers, preventing
// unsafe elimination of instructions that are branch targets.
// Also populates labelsBeforeLine for reassembly output.
static void ResolveRelativeBranches(std::vector<Token>& tokens, std::map<int, std::vector<std::string>>& labelsBeforeLine)
{
	int labelCounter = 0;

	struct Resolution
	{
		size_t branchIdx;
		size_t targetIdx;
		std::string label;
	};
	std::vector<Resolution> resolutions;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].type != TokenType::Instruction)
			continue;
		if (!IsConditionalBranch(tokens[i].mnemonic))
			continue;

		int offset;
		bool forward;
		if (!ParseRelativeOffset(tokens[i].operand, offset, forward))
			continue;

		if (forward)
		{
			// *+N: target is N bytes from the start of this branch instruction.
			// The branch itself is 2 bytes, so skip N-2 more bytes of instructions.
			int remaining = offset - 2;
			if (remaining <= 0)
				continue;

			size_t target = i + 1;
			bool valid = true;
			while (target < tokens.size() && remaining > 0)
			{
				if (tokens[target].type == TokenType::Instruction)
				{
					int sz = EstimateInstructionSize(tokens[target].mnemonic, tokens[target].operand);
					remaining -= sz;
					if (remaining < 0)
					{
						valid = false;
						break;
					}
				}
				target++;
			}

			if (valid && remaining == 0 && target <= tokens.size())
			{
				char buf[32];
				snprintf(buf, sizeof(buf), "_opt_br_%d", labelCounter++);
				Resolution r;
				r.branchIdx = i;
				r.targetIdx = target;
				r.label = buf;
				resolutions.push_back(r);
			}
		}
		else
		{
			// *-N: target is N bytes before the start of this branch instruction.
			// Walk backward through preceding instructions, summing sizes.
			int remaining = offset;
			if (remaining <= 0 || i == 0)
				continue;

			size_t target = i;
			bool valid = true;
			while (target > 0 && remaining > 0)
			{
				target--;
				if (tokens[target].type == TokenType::Instruction)
				{
					int sz = EstimateInstructionSize(tokens[target].mnemonic, tokens[target].operand);
					remaining -= sz;
					if (remaining < 0)
					{
						valid = false;
						break;
					}
				}
			}

			if (valid && remaining == 0)
			{
				char buf[32];
				snprintf(buf, sizeof(buf), "_opt_br_%d", labelCounter++);
				Resolution r;
				r.branchIdx = i;
				r.targetIdx = target;
				r.label = buf;
				resolutions.push_back(r);
			}
		}
	}

	if (resolutions.empty())
		return;

	// Update branch operands to use labels
	for (size_t r = 0; r < resolutions.size(); r++)
	{
		tokens[resolutions[r].branchIdx].operand = resolutions[r].label;
		tokens[resolutions[r].branchIdx].text = tokens[resolutions[r].branchIdx].mnemonic + " " + resolutions[r].label;
	}

	// Populate labelsBeforeLine for reassembly
	for (size_t r = 0; r < resolutions.size(); r++)
	{
		int targetLineIdx;
		if (resolutions[r].targetIdx < tokens.size())
			targetLineIdx = tokens[resolutions[r].targetIdx].lineIndex;
		else
			targetLineIdx = tokens.back().lineIndex + 1;
		labelsBeforeLine[targetLineIdx].push_back(resolutions[r].label);
	}

	// Insert label tokens into the stream (as barriers for the optimizer).
	// Sort by targetIdx descending so insertions don't shift pending indices.
	std::sort(resolutions.begin(), resolutions.end(),
		[](const Resolution& a, const Resolution& b) { return a.targetIdx > b.targetIdx; });

	for (size_t r = 0; r < resolutions.size(); r++)
	{
		Token labelTok;
		labelTok.type = TokenType::Label;
		labelTok.text = resolutions[r].label;
		labelTok.eliminated = false;
		labelTok.lineIndex = -1;  // synthetic, handled separately in reassembly
		tokens.insert(tokens.begin() + resolutions[r].targetIdx, labelTok);
	}

	if (g_verbosity >= 3 && labelCounter > 0)
	{
		printf("MacroSplitter: Resolved %d relative branch target%s to labels\n",
			labelCounter, (labelCounter != 1) ? "s" : "");
	}
}


// Try to evaluate a simple numeric expression wrapped in <() or >() operators.
// Returns true if the expression is a pure numeric literal, with the result in 'value'.
static bool TryEvalHiLoExpression(const std::string& expr, int& value)
{
	// Check for <(expr) or >(expr)
	if (expr.size() < 4)
		return false;
	char op = expr[0];
	if (op != '<' && op != '>')
		return false;
	if (expr[1] != '(' || expr[expr.size() - 1] != ')')
		return false;

	std::string inner = expr.substr(2, expr.size() - 3);
	long num = 0;
	if (!TryParseAddress(inner, num))
		return false;

	if (op == '<')
		value = (int)(num & 0xFF);
	else
		value = (int)((num >> 8) & 0xFF);
	return true;
}


// Normalize immediate operands containing <() and >() with numeric literals.
// e.g. "#<(590)" -> "#70", "#>(590)" -> "#2", "#<(0)" -> "#0", "#>(0)" -> "#0"
// This enables the optimizer to detect redundant loads of the same effective value.
static void NormalizeImmediateOperands(std::vector<Token>& tokens)
{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].type != TokenType::Instruction)
			continue;
		if (tokens[i].operand.empty() || tokens[i].operand[0] != '#')
			continue;

		std::string imm = tokens[i].operand.substr(1);  // strip '#'
		int value;
		if (TryEvalHiLoExpression(imm, value))
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "#%d", value);
			tokens[i].operand = buf;
			tokens[i].text = tokens[i].mnemonic + " " + tokens[i].operand;
		}
	}
}


// Store/Load pairs for pattern matching
struct RegisterPair
{
	const char* load;
	const char* store;
};

static const RegisterPair g_registerPairs[] =
{
	{ "lda", "sta" },
	{ "ldx", "stx" },
	{ "ldy", "sty" }
};


// Cross-register transfer pairs: stX -> ldA can become tXa
struct CrossRegisterTransfer
{
	const char* store;      // e.g. "stx"
	const char* load;       // e.g. "lda"
	const char* transfer;   // e.g. "txa"
};

static const CrossRegisterTransfer g_crossTransfers[] =
{
	{ "sta", "ldx", "tax" },
	{ "sta", "ldy", "tay" },
	{ "stx", "lda", "txa" },
	{ "sty", "lda", "tya" }
};


static int RunOptimizationPass(std::vector<Token>& tokens)
{
	int eliminatedCount = 0;

	for (size_t i = 0; i + 1 < tokens.size(); i++)
	{
		Token& a = tokens[i];
		Token& b = tokens[i + 1];

		// Skip already eliminated tokens or non-instructions
		if (a.eliminated || b.eliminated)
			continue;
		if (a.type != TokenType::Instruction || b.type != TokenType::Instruction)
			continue;

		// Check if b is a barrier (shouldn't happen for instructions, but be safe)
		if (IsBarrier(b.type))
			continue;

		for (int p = 0; p < 3; p++)
		{
			const RegisterPair& rp = g_registerPairs[p];

			// Pattern 1: Self-store elimination (load X : store X -> remove store)
			if (a.mnemonic == rp.load && b.mnemonic == rp.store && a.operand == b.operand)
			{
				b.eliminated = true;
				eliminatedCount++;
				if (g_verbosity >= 3)
				{
					printf("MacroSplitter: [line %d] Self-store eliminated: %s\n", b.lineIndex + 1, b.text.c_str());
				}
				break;
			}

			// Pattern 2: Load-after-store elimination (store X : load X -> remove load)
			if (a.mnemonic == rp.store && b.mnemonic == rp.load && a.operand == b.operand)
			{
				b.eliminated = true;
				eliminatedCount++;
				if (g_verbosity >= 3)
				{
					printf("MacroSplitter: [line %d] Load-after-store eliminated: %s\n", b.lineIndex + 1, b.text.c_str());
				}
				break;
			}
		}
	}

	return eliminatedCount;
}


static int OptimizeBuffer(std::string& buffer, int& bytesSaved)
{
	bytesSaved = 0;
	// Normalize line endings: strip \r, work with \n internally
	std::string normalized;
	normalized.reserve(buffer.size());
	for (size_t i = 0; i < buffer.size(); i++)
	{
		if (buffer[i] != '\r')
			normalized += buffer[i];
	}

	// Split into lines
	std::vector<std::string> lines;
	{
		size_t start = 0;
		while (start <= normalized.size())
		{
			size_t pos = normalized.find('\n', start);
			if (pos == std::string::npos)
			{
				lines.push_back(normalized.substr(start));
				break;
			}
			lines.push_back(normalized.substr(start, pos - start));
			start = pos + 1;
		}
	}

	// Tokenize all lines into a flat stream
	std::vector<Token> allTokens;
	for (int i = 0; i < (int)lines.size(); i++)
	{
		std::vector<Token> lineTokens = TokenizeLine(lines[i], i);
		for (size_t j = 0; j < lineTokens.size(); j++)
			allTokens.push_back(lineTokens[j]);
	}

	// Resolve *+N / *-N branch targets to synthetic labels (creates barriers)
	std::map<int, std::vector<std::string>> labelsBeforeLine;
	ResolveRelativeBranches(allTokens, labelsBeforeLine);

	// Normalize #<(N) / #>(N) immediates to plain numbers for matching
	NormalizeImmediateOperands(allTokens);

	// Run optimization passes until no more changes
	int totalEliminated = 0;
	for (;;)
	{
		int eliminated = 0;

		for (size_t i = 0; i < allTokens.size(); i++)
		{
			if (allTokens[i].eliminated)
				continue;

			// Find the next non-eliminated token
			size_t j = i + 1;
			while (j < allTokens.size() && allTokens[j].eliminated)
				j++;

			if (j >= allTokens.size())
				break;

			Token& a = allTokens[i];
			Token& b = allTokens[j];

			// If either is a barrier, skip
			if (IsBarrier(a.type) || IsBarrier(b.type))
				continue;
			if (a.type != TokenType::Instruction || b.type != TokenType::Instruction)
				continue;

			for (int p = 0; p < 3; p++)
			{
				const RegisterPair& rp = g_registerPairs[p];

				// Pattern 1: Self-store elimination
				if (a.mnemonic == rp.load && b.mnemonic == rp.store && a.operand == b.operand)
				{
					int bytes = EstimateInstructionSize(b.mnemonic, b.operand);
					b.eliminated = true;
					eliminated++;
					bytesSaved += bytes;
					if (g_verbosity >= 3)
					{
						printf("MacroSplitter: [line %d] Self-store eliminated: %s (%d bytes)\n", b.lineIndex + 1, b.text.c_str(), bytes);
					}
					break;
				}

				// Pattern 2: Load-after-store elimination
				if (a.mnemonic == rp.store && b.mnemonic == rp.load && a.operand == b.operand)
				{
					int bytes = EstimateInstructionSize(b.mnemonic, b.operand);
					b.eliminated = true;
					eliminated++;
					bytesSaved += bytes;
					if (g_verbosity >= 3)
					{
						printf("MacroSplitter: [line %d] Load-after-store eliminated: %s (%d bytes)\n", b.lineIndex + 1, b.text.c_str(), bytes);
					}
					break;
				}

				// Pattern 3: Dead store elimination (store X : store X -> remove first store)
				// Safe because store instructions don't modify the accumulator or flags.
				// Exception: writes to Oric I/O page ($300-$3FF) have hardware side effects.
				if (a.mnemonic == rp.store && b.mnemonic == rp.store && a.operand == b.operand
					&& !IsIOPageAddress(a.operand))
				{
					int bytes = EstimateInstructionSize(a.mnemonic, a.operand);
					a.eliminated = true;
					eliminated++;
					bytesSaved += bytes;
					if (g_verbosity >= 3)
					{
						printf("MacroSplitter: [line %d] Dead store eliminated: %s (%d bytes)\n", a.lineIndex + 1, a.text.c_str(), bytes);
					}
					break;
				}
			}

			// Pattern 4: Tail call optimization (jsr X : rts -> jmp X, remove rts)
			// Safe because resolved branch labels act as barriers if the rts is a branch target.
			if (a.mnemonic == "jsr" && b.mnemonic == "rts")
			{
				a.mnemonic = "jmp";
				a.text = "jmp " + a.operand;
				b.eliminated = true;
				eliminated++;
				bytesSaved += 1;  // rts = 1 byte
				if (g_verbosity >= 3)
				{
					printf("MacroSplitter: [line %d] Tail call: jsr %s -> jmp (1 byte)\n", a.lineIndex + 1, a.operand.c_str());
				}
			}

			// Pattern 5: Dead code after unconditional jump (jmp X : rts -> remove rts)
			// Safe because resolved branch labels act as barriers if the rts is a branch target.
			if (a.mnemonic == "jmp" && b.mnemonic == "rts")
			{
				b.eliminated = true;
				eliminated++;
				bytesSaved += 1;  // rts = 1 byte
				if (g_verbosity >= 3)
				{
					printf("MacroSplitter: [line %d] Dead code after jmp eliminated: rts (1 byte)\n", b.lineIndex + 1);
				}
			}

			// Pattern 6: Cross-register transfer (stx X ... lda X -> replace lda with txa, etc.)
			// When a value was stored from one register to address X, and we later load X
			// into a different register, we can use a transfer instruction (1 byte vs 2-3).
			// Scan backward from b, skipping stores to different addresses (they don't
			// modify any register or the target memory).
			if ((b.mnemonic == "lda" || b.mnemonic == "ldx" || b.mnemonic == "ldy")
				&& !b.operand.empty() && b.operand[0] != '#' && b.operand[0] != '(')
			{
				size_t scanPos = j;
				int scanCount = 0;
				bool foundTransfer = false;

				while (scanPos > 0 && scanCount < 4)
				{
					scanPos--;
					if (allTokens[scanPos].eliminated)
						continue;
					if (IsBarrier(allTokens[scanPos].type))
						break;
					if (allTokens[scanPos].type != TokenType::Instruction)
						break;

					Token& s = allTokens[scanPos];

					// Check if this is a matching cross-register store
					for (int ct = 0; ct < 4; ct++)
					{
						const CrossRegisterTransfer& xr = g_crossTransfers[ct];
						if (s.mnemonic == xr.store && b.mnemonic == xr.load && s.operand == b.operand)
						{
							int oldBytes = EstimateInstructionSize(b.mnemonic, b.operand);
							int saved = oldBytes - 1;
							b.mnemonic = xr.transfer;
							b.operand = "";
							b.text = xr.transfer;
							eliminated++;
							bytesSaved += saved;
							if (g_verbosity >= 3)
							{
								printf("MacroSplitter: [line %d] Cross-register transfer: %s %s -> %s (%d byte%s)\n",
									b.lineIndex + 1, xr.load, s.operand.c_str(), xr.transfer, saved, (saved != 1) ? "s" : "");
							}
							foundTransfer = true;
							break;
						}
					}
					if (foundTransfer)
						break;

					// Only allow stores to different addresses as safe intervening instructions
					bool isStore = (s.mnemonic == "sta" || s.mnemonic == "stx" || s.mnemonic == "sty");
					if (!isStore || s.operand == b.operand)
						break;

					scanCount++;
				}
			}

			// Pattern 7: Same-immediate reload (lda #X : sta Y : lda #X -> remove second lda)
			// After lda #X : sta Y, the accumulator still holds #X (sta doesn't modify A or flags).
			// So a subsequent lda #X is redundant.
			// Also works with ldx #X : stx Y : ldx #X and ldy #Y : sty Z : ldy #Y.
			if (a.mnemonic[0] == 's' && b.mnemonic[0] == 'l' && a.operand != b.operand)
			{
				// a is a store, b is a load of something different (otherwise Pattern 2 handles it)
				// Look backward for the instruction before a
				size_t prev = i;
				while (prev > 0)
				{
					prev--;
					if (!allTokens[prev].eliminated)
						break;
				}
				if (prev < i && !allTokens[prev].eliminated
					&& allTokens[prev].type == TokenType::Instruction
					&& !IsBarrier(allTokens[prev].type))
				{
					Token& z = allTokens[prev];
					// Check: z loads an immediate, a stores with the SAME register, b reloads same immediate
					for (int p = 0; p < 3; p++)
					{
						const RegisterPair& rp = g_registerPairs[p];
						if (z.mnemonic == rp.load && a.mnemonic == rp.store
							&& b.mnemonic == rp.load
							&& z.operand[0] == '#' && z.operand == b.operand)
						{
							int bytes = EstimateInstructionSize(b.mnemonic, b.operand);
							b.eliminated = true;
							eliminated++;
							bytesSaved += bytes;
							if (g_verbosity >= 3)
							{
								printf("MacroSplitter: [line %d] Same-immediate reload eliminated: %s (%d byte%s)\n",
									b.lineIndex + 1, b.text.c_str(), bytes, (bytes != 1) ? "s" : "");
							}
							break;
						}
					}
				}
			}
		}

		totalEliminated += eliminated;
		if (eliminated == 0)
			break;
	}

	// Reassemble: emit each surviving token on its own line.
	// This is critical because some macros use ':' separators (not ';'), so multiple
	// instructions may have originated from the same source line. If we rejoin them
	// with ':', link65's strtok-based parser can miss label references when an
	// optimizer-introduced instruction (like txa) causes strtok to scan past ':'
	// boundaries, corrupting the outer strchr loop. One token per line avoids this.
	// Labels go at column 0; instructions get the original line's indentation.
	std::string output;
	output.reserve(buffer.size());

	for (int lineIdx = 0; lineIdx < (int)lines.size(); lineIdx++)
	{
		// Emit any synthetic labels that belong before this line
		std::map<int, std::vector<std::string>>::iterator it = labelsBeforeLine.find(lineIdx);
		if (it != labelsBeforeLine.end())
		{
			for (size_t l = 0; l < it->second.size(); l++)
			{
				output += it->second[l] + "\r\n";
			}
		}

		// Get the indentation of the original line
		std::string indent;
		for (size_t c = 0; c < lines[lineIdx].size(); c++)
		{
			if (lines[lineIdx][c] == ' ' || lines[lineIdx][c] == '\t')
				indent += lines[lineIdx][c];
			else
				break;
		}

		// Collect non-eliminated tokens for this line (skip synthetic labels with lineIndex == -1)
		bool hasAny = false;
		for (size_t t = 0; t < allTokens.size(); t++)
		{
			if (allTokens[t].lineIndex == lineIdx && !allTokens[t].eliminated)
			{
				hasAny = true;
				// Labels and directives go at column 0; instructions get the indentation
				if (allTokens[t].type == TokenType::Label)
					output += allTokens[t].text + "\r\n";
				else
					output += indent + allTokens[t].text + "\r\n";
			}
		}

		if (!hasAny)
		{
			output += "\r\n";
		}
	}

	// Emit any synthetic labels that go after the last line
	std::map<int, std::vector<std::string>>::iterator it = labelsBeforeLine.find((int)lines.size());
	if (it != labelsBeforeLine.end())
	{
		for (size_t l = 0; l < it->second.size(); l++)
		{
			output += it->second[l] + "\r\n";
		}
	}

	buffer = output;
	return totalEliminated;
}


// ============================================================================
// Built-in Macro Expander (replaces cpp.exe macro expansion step)
// Activated with the -M flag: macrosplitter -Mmacros.h input.c2 output
// ============================================================================

struct MacroDef
{
	std::string              name;
	std::vector<std::string> params;   // empty for object-like macros
	std::string              body;     // raw body text (separators intact)
	bool                     isFunctionLike;
};

static std::map<std::string, MacroDef> g_macros;


static bool IsIdentStart(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}


static bool IsIdentChar(char c)
{
	return IsIdentStart(c) || (c >= '0' && c <= '9');
}


static std::string ExtractIdentifier(const std::string& text, size_t& pos)
{
	size_t start = pos;
	while (pos < text.size() && IsIdentChar(text[pos]))
		pos++;
	return text.substr(start, pos - start);
}


// Parse comma-separated arguments from a macro call with parenthesis balancing.
// pos starts AFTER the opening '(', returns with pos AFTER the closing ')'.
static std::vector<std::string> ParseMacroCallArgs(const std::string& text, size_t& pos)
{
	std::vector<std::string> args;
	int depth = 1;
	std::string current;

	while (pos < text.size() && depth > 0)
	{
		char c = text[pos];
		if (c == '(')
		{
			depth++;
			current += c;
		}
		else if (c == ')')
		{
			depth--;
			if (depth == 0)
			{
				args.push_back(TrimString(current));
				pos++;
				return args;
			}
			current += c;
		}
		else if (c == ',' && depth == 1)
		{
			args.push_back(TrimString(current));
			current.clear();
		}
		else
		{
			current += c;
		}
		pos++;
	}

	if (!current.empty())
		args.push_back(TrimString(current));
	return args;
}


// Substitute parameter names with argument values (whole-word matching only).
static std::string SubstituteParams(const std::string& body,
	const std::vector<std::string>& paramNames,
	const std::vector<std::string>& argValues)
{
	if (paramNames.empty())
		return body;

	std::string result;
	result.reserve(body.size() * 2);
	size_t pos = 0;

	while (pos < body.size())
	{
		if (IsIdentStart(body[pos]))
		{
			std::string ident = ExtractIdentifier(body, pos);
			bool matched = false;
			for (size_t p = 0; p < paramNames.size(); p++)
			{
				if (ident == paramNames[p] && p < argValues.size())
				{
					result += argValues[p];
					matched = true;
					break;
				}
			}
			if (!matched)
				result += ident;
		}
		else
		{
			result += body[pos];
			pos++;
		}
	}
	return result;
}


// Recursively expand all macro references in text.
static std::string ExpandText(const std::string& text, int depth)
{
	if (depth > 16)
		return text;

	std::string result;
	result.reserve(text.size() * 2);
	size_t pos = 0;
	bool changed = false;

	while (pos < text.size())
	{
		if (IsIdentStart(text[pos]))
		{
			std::string ident = ExtractIdentifier(text, pos);
			std::map<std::string, MacroDef>::const_iterator it = g_macros.find(ident);
			if (it != g_macros.end())
			{
				const MacroDef& macro = it->second;
				if (macro.isFunctionLike)
				{
					// Check for '(' after optional whitespace
					size_t ahead = pos;
					while (ahead < text.size() && (text[ahead] == ' ' || text[ahead] == '\t'))
						ahead++;

					if (ahead < text.size() && text[ahead] == '(')
					{
						ahead++;
						std::vector<std::string> args = ParseMacroCallArgs(text, ahead);
						std::string substituted = SubstituteParams(macro.body, macro.params, args);
						result += substituted;
						pos = ahead;
						changed = true;
					}
					else
					{
						// Function-like macro without args - keep as-is
						result += ident;
					}
				}
				else
				{
					// Object-like macro: replace with body
					result += macro.body;
					changed = true;
				}
			}
			else
			{
				result += ident;
			}
		}
		else
		{
			result += text[pos];
			pos++;
		}
	}

	if (changed)
		return ExpandText(result, depth + 1);
	return result;
}


static bool LoadMacroFile(const std::string& filePath)
{
	void* pBuffer = 0;
	size_t nSize = 0;
	if (!LoadFile(filePath.c_str(), pBuffer, nSize))
	{
		printf("MacroSplitter: ERROR - Cannot load macro file: %s\n", filePath.c_str());
		return false;
	}

	std::string content((const char*)pBuffer, nSize);
	free(pBuffer);

	// Split into lines
	std::vector<std::string> lines;
	{
		size_t start = 0;
		while (start <= content.size())
		{
			size_t pos = content.find('\n', start);
			if (pos == std::string::npos)
			{
				std::string line = content.substr(start);
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				lines.push_back(line);
				break;
			}
			std::string line = content.substr(start, pos - start);
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			lines.push_back(line);
			start = pos + 1;
		}
	}

	// Process lines: join continuations and parse #define directives
	size_t i = 0;
	while (i < lines.size())
	{
		std::string line = lines[i];

		// Join continuation lines (ending with backslash)
		while (!line.empty() && line[line.size() - 1] == '\\' && i + 1 < lines.size())
		{
			line.erase(line.size() - 1);
			i++;
			line += lines[i];
		}
		i++;

		std::string trimmed = TrimString(line);
		if (trimmed.empty())
			continue;

		// Skip C comments
		if (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '*')
			continue;

		// Only process #define directives; skip #ifdef, #endif, etc.
		if (trimmed[0] != '#')
			continue;
		if (trimmed.size() < 7 || trimmed.substr(0, 7) != "#define")
			continue;

		// Parse: #define NAME[(params)] [body]
		size_t pos = 7;
		while (pos < trimmed.size() && (trimmed[pos] == ' ' || trimmed[pos] == '\t'))
			pos++;
		if (pos >= trimmed.size())
			continue;

		std::string macroName = ExtractIdentifier(trimmed, pos);
		if (macroName.empty())
			continue;

		MacroDef def;
		def.name = macroName;
		def.isFunctionLike = false;

		// Function-like: '(' immediately after name (no space)
		if (pos < trimmed.size() && trimmed[pos] == '(')
		{
			def.isFunctionLike = true;
			pos++;
			while (pos < trimmed.size() && trimmed[pos] != ')')
			{
				while (pos < trimmed.size() && (trimmed[pos] == ' ' || trimmed[pos] == '\t' || trimmed[pos] == ','))
					pos++;
				if (pos < trimmed.size() && trimmed[pos] != ')')
				{
					std::string param = ExtractIdentifier(trimmed, pos);
					if (!param.empty())
						def.params.push_back(param);
				}
			}
			if (pos < trimmed.size() && trimmed[pos] == ')')
				pos++;
		}

		// Skip whitespace before body
		while (pos < trimmed.size() && (trimmed[pos] == ' ' || trimmed[pos] == '\t'))
			pos++;

		def.body = (pos < trimmed.size()) ? trimmed.substr(pos) : "";
		g_macros[macroName] = def;
	}

	if (g_verbosity >= 2)
	{
		printf("MacroSplitter: Loaded %d macro definitions from %s\n",
			(int)g_macros.size(), filePath.c_str());
	}
	return true;
}


// Expand a .c2 file: expand macros and split into one instruction per line.
static std::string ExpandC2File(const std::string& input)
{
	std::string result;
	result.reserve(input.size() * 4);

	// Split into lines
	std::vector<std::string> lines;
	{
		size_t start = 0;
		while (start <= input.size())
		{
			size_t pos = input.find('\n', start);
			if (pos == std::string::npos)
			{
				std::string line = input.substr(start);
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				lines.push_back(line);
				break;
			}
			std::string line = input.substr(start, pos - start);
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			lines.push_back(line);
			start = pos + 1;
		}
	}

	for (size_t lineIdx = 0; lineIdx < lines.size(); lineIdx++)
	{
		const std::string& line = lines[lineIdx];
		std::string trimmed = TrimString(line);

		// Empty line
		if (trimmed.empty())
		{
			result += "\r\n";
			continue;
		}

		// C comment
		if (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '*')
		{
			result += "; " + trimmed + "\r\n";
			continue;
		}

		// Label at column 0 (not indented)
		bool isIndented = (!line.empty() && (line[0] == ' ' || line[0] == '\t'));
		if (!isIndented)
		{
			result += trimmed + "\r\n";
			continue;
		}

		// Indented line: expand macro call
		std::string expanded = ExpandText(trimmed, 0);
		bool wasExpanded = (expanded != trimmed);

		if (!wasExpanded)
		{
			// Not a known macro - pass through as-is
			result += "\t" + trimmed + "\r\n";
			continue;
		}

		// Split expanded text on ':' and ';' into individual instructions
		std::vector<std::string> instructions;
		{
			size_t start = 0;
			while (start <= expanded.size())
			{
				size_t sepPos = std::string::npos;
				for (size_t s = start; s < expanded.size(); s++)
				{
					if (expanded[s] == ':' || expanded[s] == ';')
					{
						sepPos = s;
						break;
					}
				}

				std::string segment;
				if (sepPos == std::string::npos)
				{
					segment = expanded.substr(start);
					start = expanded.size() + 1;
				}
				else
				{
					segment = expanded.substr(start, sepPos - start);
					start = sepPos + 1;
				}

				std::string seg = TrimString(segment);
				if (!seg.empty())
					instructions.push_back(seg);
			}
		}

		if (instructions.empty())
			continue;

		// Emit annotation comment showing original macro call
		result += "; === " + trimmed + " ===\r\n";

		// Emit each instruction: labels at column 0, everything else indented
		for (size_t inst = 0; inst < instructions.size(); inst++)
		{
			Token tok = ParseToken(instructions[inst], 0);
			if (tok.type == TokenType::Label)
				result += instructions[inst] + "\r\n";
			else
				result += "\t" + instructions[inst] + "\r\n";
		}
	}

	return result;
}


#define NB_ARG	2


/**
 * argv[1] - Original filename (macro converted file)
 * argv[2] - Destination filename (tape file)
 */
int main(int argc,char *argv[])
{
	//
	// Some initialization for the common library
	//
	SetApplicationParameters(
		"MacroSplitter",
		TOOL_VERSION_MAJOR,
		TOOL_VERSION_MINOR,
		"{ApplicationName} - Version {ApplicationVersion} - This program is a part of the OSDK (http://www.osdk.org)\r\n"
		"\r\n"
		"Author:\r\n"
		"  Mickael Pointier (aka Dbug)\r\n"
		"  dbug@defence-force.org\r\n"
		"  http://www.defence-force.org\r\n"
		"\r\n"
		"Purpose:\r\n"
		"  Reformat the file resulting from the macro conversion by splitting the\r\n"
		"  lines after the ; character\r\n"
		"\r\n"
		"Parameters:\r\n"
		"  <options> <sourcefile> <destinationfile>\r\n"
		"\r\n"
		"Switches:\r\n"
		"  -O               Enable 6502 peephole optimization\r\n"
		"  -M<macrofile>    Expand macros from <macrofile> (replaces cpp.exe step)\r\n"
		"\r\n"
		"Exemple:\r\n"
		"  {ApplicationName} source.s destination.s\r\n"
		"  {ApplicationName} -O source.s destination.s\r\n"
		"  {ApplicationName} -Mmacros.h source.c2 destination.s\r\n"
		"  {ApplicationName} -O -Mmacros.h source.c2 destination.s\r\n"
		);

	//
	// Read verbosity from environment
	//
	const char* verbEnv = getenv("OSDKVERBOSITY");
	if (verbEnv)
	{
		g_verbosity = atoi(verbEnv);
	}

	//
	// Parse arguments
	//
	bool flag_optimize = false;
	std::string macroFilePath;

	ArgumentParser argumentParser(argc, argv);
	while (argumentParser.ProcessNextArgument())
	{
		if (argumentParser.IsSwitch("-O"))
		{
			flag_optimize = true;
		}
		else if (argumentParser.IsSwitch("-M"))
		{
			macroFilePath = argumentParser.GetStringValue();
		}
	}

	if (argumentParser.GetParameterCount() != NB_ARG)
	{
		//
		// Wrong number of arguments
		//
		ShowError(0);
	}


	//
	// Read file
	//
	void *pBuffer;
	size_t nSizeBuffer;
	if (!LoadFile(argumentParser.GetParameter(0),pBuffer,nSizeBuffer))
	{
		ShowError("unable to load source file");
	}

	if (!macroFilePath.empty())
	{
		//
		// New pipeline: load macros, expand .c2 input, split, optimize, save
		//
		if (!LoadMacroFile(macroFilePath))
		{
			ShowError("unable to load macro file");
		}

		std::string input((const char*)pBuffer, nSizeBuffer);
		free(pBuffer);
		pBuffer = 0;

		std::string expanded = ExpandC2File(input);

		if (flag_optimize)
		{
			int bytesSaved = 0;
			int eliminated = OptimizeBuffer(expanded, bytesSaved);
			if (g_verbosity >= 1 && eliminated > 0)
			{
				printf("MacroSplitter: Optimizer removed %d redundant instruction%s (saving %d byte%s)\n",
					eliminated, (eliminated != 1) ? "s" : "", bytesSaved, (bytesSaved != 1) ? "s" : "");
			}
		}

		nSizeBuffer = expanded.size();
		pBuffer = malloc(nSizeBuffer + 1);
		memcpy(pBuffer, expanded.c_str(), nSizeBuffer + 1);
	}
	else
	{
		//
		// Old pipeline: split on semicolons, optimize
		//
		if (!ConvertBuffer(pBuffer,nSizeBuffer))
		{
			ShowError("unable to convert data");
		}

		if (flag_optimize)
		{
			std::string bufferStr((const char*)pBuffer, nSizeBuffer);
			free(pBuffer);

			int bytesSaved = 0;
			int eliminated = OptimizeBuffer(bufferStr, bytesSaved);
			if (g_verbosity >= 1 && eliminated > 0)
			{
				printf("MacroSplitter: Optimizer removed %d redundant instruction%s (saving %d byte%s)\n",
					eliminated, (eliminated != 1) ? "s" : "", bytesSaved, (bytesSaved != 1) ? "s" : "");
			}

			nSizeBuffer = bufferStr.size();
			pBuffer = malloc(nSizeBuffer + 1);
			memcpy(pBuffer, bufferStr.c_str(), nSizeBuffer + 1);
		}
	}

	//
	// Write file
	//
	if (!SaveFile(argumentParser.GetParameter(1),pBuffer,nSizeBuffer))
	{
		ShowError("unable to create destination file");
	}

	free(pBuffer);

	exit(0);
}
