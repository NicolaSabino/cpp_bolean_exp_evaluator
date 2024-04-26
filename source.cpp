#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <stack>
#include <regex>
#include <cassert>

// Bottom-up parsing
// https://www.youtube.com/watch?v=tH5AOX9929g
// https://www.meziantou.net/creating-a-parser-for-boolean-expressions.htm
// https://www2.lawrence.edu/fast/GREGGJ/CMSC270/parser/parser.html

enum class TokenType
{
    T_LX_PAREN,
    T_RX_PAREN,
    T_NUMBER,
    T_ID,
    T_PREFIX_OP,
    T_INFIX_OP,
    T_END,
    T_BOOL
};

struct Token
{
    TokenType type;
    std::string value;
};

/**
 * @brief Tokenize string
 *
 * String lexical analysis
 *
 * Convert `input_string` into a vector of token
 *
 * It always return a vector of token.
 *
 * @param input_string The input string.
 * @return std::vector<Token> The vector of token.
 */
std::vector<Token> tokenize(const std::string &input_string)
{
    std::vector<Token> tokens;
    std::string buffer; // To store characters temporarily


    for (char c : input_string)
    {

        if (std::isdigit(c) || c == '.')
            buffer += c;
        else if (buffer.empty())
        {
            if (c == '!')
                tokens.push_back({TokenType::T_PREFIX_OP, std::string{c}});
            else if (c == '(')
                tokens.push_back({TokenType::T_LX_PAREN, std::string{c}});
            else if (c == ')')
                tokens.push_back({TokenType::T_RX_PAREN, std::string{c}});
            else if (c == '>' || c == '<') // we do not consider >= and <= operators
                tokens.push_back({TokenType::T_INFIX_OP, std::string{c}});
            else if (c == '&' || c == '|' || c == '=' || c == 'v' || c == '-' || c == '>' || c == '<' || c == 't' || c == 'f')
                buffer += c;
        }
        else // buffer not empty
        {
            if (std::isdigit(buffer.back()) && c == ')')
            {
                // special case  for bug related to val 3 in e6
                if (buffer.at(0) == 'v' && buffer.size() >= 2)
                    tokens.push_back({TokenType::T_ID, buffer}); 
                else
                    tokens.push_back({TokenType::T_NUMBER, buffer});
                tokens.push_back({TokenType::T_RX_PAREN, std::string{c}});
            }
            else if (buffer.at(0) == 'v' && buffer.size() >= 2)
                tokens.push_back({TokenType::T_ID, buffer});
            else if (buffer.back() == '|')
                tokens.push_back({TokenType::T_INFIX_OP, "||"});
            else if (buffer.back() == '&' && c == '&')
                tokens.push_back({TokenType::T_INFIX_OP, "&&"});
            else if (buffer.back() == '=' && c == '=')
                tokens.push_back({TokenType::T_INFIX_OP, "=="});
            else if (c == 'e')
            {
                buffer += c;
                tokens.push_back({TokenType::T_BOOL, buffer});
            }
            else if (c == ' ')
                tokens.push_back({TokenType::T_NUMBER, buffer});
            else if (buffer.at(0) == 't' || buffer.at(0) == 'f')
            {
                buffer += c;
                continue;
            }
                
            buffer.clear();
        }
    }

    if (!buffer.empty())
    {
        if (buffer.at(0) == 'v' && buffer.size() >= 2)
            tokens.push_back({TokenType::T_ID, buffer});
        else
            tokens.push_back({TokenType::T_NUMBER, buffer});
        buffer.clear();
    }

    // tokens.push_back({TokenType::T_END, ""});

    return tokens;
}

/**
 * @brief Sanity check for `tokenize()` function
 */
void tokenizeSanityCheck()
{
    std::string e6 = "((v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && v4) && (v5 == !v4) ";
    const auto res = tokenize(e6);

    assert(res[0].value == "(");
    assert(res[1].value == "(");
    assert(res[5].value == "||");
    assert(res[15].value == ")");
    assert(res[19].value == "-15.000000001");
    assert(res[20].value == "&&");
    assert(res[27].value == "!");
}

class Parser
{
public:
    Parser(const std::string &expression, const std::map<std::string, std::string> values)
        : expression(expression), values(values){};
    bool parse();

private:
    /**
     * @brief Reduce token buffer
     *
     * @return true
     * @return false
     */
    void reduce();

    /**
     * @brief Check if last token is comparisson
     *
     * Check if last 3 token in token buffer is
     * coparisson.
     *
     * @return true If last 3 elements are values and operator.
     * @return false If buffer size is too small or if there is no comparisson.
     */
    bool isNumberComparisson();

    bool isBooleanComparisson();
    bool isNegation();
    bool isBooleanVariable();
    /**
     * @brief Compare operation
     *
     * Reduce a comparison operation to a
     * `true/false` value
     *
     * It automatically fetch the last 3 elements in Parser::tokenBuffer
     *
     * @see Parser::reduce
     */
    void compareNumbers();
    void compareBooleans();
    void negate();
    void evaluateBoolean();

    /**
     * @brief Get value from Token
     *
     * Convert a number or id into
     * corresponding float value
     *
     * @param t The input token
     * @param values The hasmap of vlaues
     * @return const float The value extracted from token.
     */
    const float getValue(const Token &t);

    static const bool isTokenVariableOrValue(const Token &t);

private:
    std::string expression;
    std::map<std::string, std::string> values;
    std::vector<Token> tokenBuffer;
};

const float Parser::getValue(const Token &t)
{
    return t.type == TokenType::T_NUMBER ? std::stof(t.value) : std::stof(this->values.at(t.value));
}

const bool Parser::isTokenVariableOrValue(const Token &t)
{
    return t.type == TokenType::T_NUMBER || t.type == TokenType::T_ID;
};

bool Parser::isNumberComparisson()
{
    const int bufferSize = this->tokenBuffer.size();
    // At least three token in the buffer
    if (bufferSize < 3)
        return false;
    // pop last three item
    const auto rightToken = this->tokenBuffer[bufferSize - 1];
    const auto opToken = this->tokenBuffer[bufferSize - 2];
    const auto leftToken = this->tokenBuffer[bufferSize - 3];
    return Parser::isTokenVariableOrValue(rightToken) && opToken.type == TokenType::T_INFIX_OP && Parser::isTokenVariableOrValue(leftToken);
};

bool Parser::isBooleanComparisson()
{
    const int bufferSize = this->tokenBuffer.size();
    // At least three token in the buffer
    if (bufferSize < 3)
        return false;
    // pop last three item
    const auto rightToken = this->tokenBuffer[bufferSize - 1];
    const auto opToken = this->tokenBuffer[bufferSize - 2];
    const auto leftToken = this->tokenBuffer[bufferSize - 3];
    const bool isLxBoolType = leftToken.type == TokenType::T_BOOL;
    const bool isRxBoolType = rightToken.type == TokenType::T_BOOL;
    const bool isOpBoolType = opToken.value == "&&" || opToken.value == "||" || opToken.value == "==";
    return isLxBoolType && isOpBoolType && isRxBoolType;
}

bool Parser::isNegation()
{
    const int bufferSize = this->tokenBuffer.size();
    // At least two token in the buffer
    if (bufferSize < 2)
        return false;
    // pop last two item
    const auto valueToken = this->tokenBuffer[bufferSize - 1];
    const auto negationToken = this->tokenBuffer[bufferSize - 2];
    const bool isNegation =  negationToken.value == "!";
    const bool isBoolValue = valueToken.type == TokenType::T_BOOL;
    return isNegation && isBoolValue;
}

void Parser::compareNumbers()
{
    const int bufferSize = this->tokenBuffer.size();
    auto rightValue = this->getValue(this->tokenBuffer[bufferSize - 1]);
    const auto op = this->tokenBuffer[bufferSize - 2];
    const auto leftValue = this->getValue(this->tokenBuffer[bufferSize - 3]);
    this->tokenBuffer.erase(this->tokenBuffer.end() - 3, this->tokenBuffer.end());

    if (op.value == "==")
    {
        const auto result = leftValue == rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
    else if (op.value == ">")
    {
        const auto result = leftValue > rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
    else if (op.value == "<")
    {
        const auto result = leftValue < rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
}

void Parser::compareBooleans()
{
    const int bufferSize = this->tokenBuffer.size();
    const auto rightToken = this->tokenBuffer[bufferSize - 1];
    const auto op = this->tokenBuffer[bufferSize - 2];
    const auto leftToken = this->tokenBuffer[bufferSize - 3];
    const bool rightValue = rightToken.value == "true";
    const bool leftValue = leftToken.value == "true";
    this->tokenBuffer.erase(this->tokenBuffer.end() - 3, this->tokenBuffer.end());

    if (op.value == "&&")
    {
        const auto result = leftValue && rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
    else if (op.value == "||")
    {
        const auto result = leftValue || rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
    else if (op.value == "==")
    {
        const auto result = leftValue == rightValue ? "true" : "false";
        this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{result}});
    }
}

void Parser::negate()
{
    const int bufferSize = this->tokenBuffer.size();
    const auto valueToken = this->tokenBuffer[bufferSize - 1];
    const bool value = valueToken.value == "true";
    this->tokenBuffer.erase(this->tokenBuffer.end() - 2, this->tokenBuffer.end());
    this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{value ? "false" : "true"}});
}

bool Parser::isBooleanVariable()
{
    const int bufferSize = this->tokenBuffer.size();
    const auto token = this->tokenBuffer[bufferSize - 1];
    if (token.type != TokenType::T_ID) return false;
    const auto stringValue = this->values.at(token.value);
    return stringValue == "true" || stringValue == "false";
}

void Parser::evaluateBoolean()
{
    const int bufferSize = this->tokenBuffer.size();
    const auto token = this->tokenBuffer[bufferSize - 1];
    const auto stringValue = this->values.at(token.value);
    this->tokenBuffer.pop_back();
    this->tokenBuffer.push_back({TokenType::T_BOOL, std::string{stringValue}});
}

void Parser::reduce()
{
    if (this->isNumberComparisson())
    {
        // compare and store result in buffer
        this->compareNumbers();
    }
    else if (this->isBooleanComparisson())
    {
        // compare bolean and store result in buffer
        this->compareBooleans();
    }
    else if (this->isNegation())
    {
        //process negation
        this->negate();
    }
    else if (this->isBooleanVariable())
    {
        //reduce to boolean
        this->evaluateBoolean();
    }
};

bool Parser::parse()
{
    const auto tokenList = tokenize(this->expression);

    for (Token token : tokenList)
    {
        // push the next token
        //  in the buffer and continue
        this->tokenBuffer.push_back(token);

        const bool isClosingBraket = token.value == ")";

        if (isClosingBraket)
            this->tokenBuffer.pop_back(); // pop ')'

        // try to reduce the buffer
        this->reduce();

        if (isClosingBraket) // pop '('
        {
            for (auto itr = this->tokenBuffer.rbegin(); itr < this->tokenBuffer.rend(); itr++) { 
        
                if ((*itr).value == "(") 
                { 
                    this->tokenBuffer.erase((itr + 1).base()); 
                    break;
                } 
            } 
        }
           // this->tokenBuffer.erase(this->tokenBuffer.end() - 2, this->tokenBuffer.end() - 1); 
    }

    while (this->tokenBuffer.size() > 1)
    {
        this->reduce();
    }
    return this->tokenBuffer.front().value == "true";
};

bool evaluate(std::string &expression, std::map<std::string, std::string> &values)
{
    Parser parser{expression, values};
    return parser.parse();
};

int main(int argc, char **argv)
{
    std::map<std::string, std::string> m;
    std::string e1 = "v0 == 1";
    std::string e2 = "(v0 == 2 || v1 > 10)";
    std::string e3 = "(v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == 0";
    std::string e4 = "(v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && !v4";
    std::string e5 = "(v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && v4";
    std::string e6 = "((v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && v4) && (v5 == !v4) ";
    std::string e7 = "true";

    tokenizeSanityCheck();

    // e1.
    // v0 == 1
    // true

    // e2.
    // (v0 == 2 || v1 > 10)
    // ( false || true )
    // true

    // e3.
    // (v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == 0
    // ( false || ( true || false )) && false
    // (false || true ) && false
    // true && false
    // false

    // e4.
    // (v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && !v4
    // ( false || ( true && false )) && false && false
    // ( false || false ) && false && false
    // false && false && false
    // false

    // e5.
    // (v0 == 2 || (v1 > 10 && v2 > 3)) && v3 == -15.000000001 && v4
    // ( false || ( true && false)) && true && true
    // ( false || false ) && true && true
    // false && true && true
    // false

    // e6.
    // ((v0 == 2 || ( v1 > 10 && v2 > 3)) && v3 == -15.000000001 && v4) && (v5 == !v4) 
    // (( false || ( true && false )) && true && true) && (false == false)
    // (( false || false ) && true && true ) && true
    // ( false && true && true ) && true
    // false && true
    // false 

    m["v0"] = "1";
    m["v1"] = "15.55";
    m["v2"] = "-10";
    m["v3"] = "-15.000000001";
    m["v4"] = "true";
    m["v5"] = "false";

    bool testCorrect =
        (evaluate(e1, m) &
             evaluate(e2, m) &
             !evaluate(e3, m) &
             !evaluate(e4, m) &
             !evaluate(e5, m) & // <-- error in the assignement
             !evaluate(e6, m) & // <-- error in the assignement
         evaluate(e7, m));

    std::cout << (testCorrect ? "Good job!" : "Uhm, please retry!") << std::endl;
};