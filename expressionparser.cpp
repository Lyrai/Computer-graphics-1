#include "expressionparser.h"

#include <QStack>
#include <cmath>

#define CHECK_ERROR if(hasError) return;

Expression Expression::invalid = Expression(false);

ExpressionParser::ExpressionParser()
{

}

Expression ExpressionParser::parse(const QString &str)
{
    lex(str);
    return parse();
}

void ExpressionParser::lex(const QString &str)
{
    int pos = 0;
    while(pos < str.size()) {
        if(str[pos].isSpace()) {
            ++pos;
            continue;
        }

        if(str[pos].isDigit()) {
            double value = 0;
            while(pos < str.size() && str[pos].isDigit()) {
                value *= 10;
                value += str[pos].digitValue();
                ++pos;
            }

            if(pos < str.size() && str[pos] == '.') {
                ++pos;
                if(pos < str.size() && str[pos].isDigit()) {
                    double fraction = 0;
                    while(str[pos].isDigit()) {
                        fraction /= 10;
                        fraction += str[pos].digitValue() / 10;
                        ++pos;
                    }

                    value += fraction;
                }
            }

            tokens.push_back(Token { TokenType::Const, value });
            continue;
        }

        if(str[pos] == '+' || str[pos] == '-' || str[pos] == '*' || str[pos] == '/' || str[pos] == '^') {
            tokens.push_back(Token { TokenType::Operation, str[pos].toLatin1() });
            ++pos;
            continue;
        }

        if(str[pos] == '(') {
            tokens.push_back(Token { TokenType::Lpar, str[pos].toLatin1() });
            ++pos;
            continue;
        }

        if(str[pos] == ')') {
            tokens.push_back(Token { TokenType::Rpar, str[pos].toLatin1() });
            ++pos;
            continue;
        }

        if(str[pos].isLetter()) {
            QString id;
            while(pos < str.size() && str[pos].isLetter()) {
                id.append(str[pos]);
                ++pos;
            }

            tokens.push_back(Token { TokenType::Id, id });
            continue;
        }

        ++pos;
    }
}

Expression ExpressionParser::parse()
{
    expression();
    if(hasError || position < tokens.size()) {
        return Expression::invalid;
    }

    return Expression(constants, operations);
}

void ExpressionParser::expression()
{
    if(position >= tokens.size()) {
        return;
    }
    if(tokens[position].type == TokenType::Lpar) {
        ++position;
        expression();
        CHECK_ERROR

        if(position >= tokens.size() || tokens[position].type != TokenType::Rpar) {
            error();
        }
        ++position;
        return;
    }

    term();
}

void ExpressionParser::term()
{
    factor();
    CHECK_ERROR

    while(position < tokens.size() && (tokens[position].value.toChar() == '+' || tokens[position].value.toChar() == '-')){
        Operation operation;
        if(tokens[position].value.toChar() == '+') {
            operation = Operation::Plus;
        } else if(tokens[position].value.toChar() == '-') {
            operation = Operation::Minus;
        }

        ++position;
        factor();

        operations.push_back(operation);
    }
}

void ExpressionParser::factor()
{
    power();
    CHECK_ERROR


    while(position < tokens.size() && (tokens[position].value.toChar() == '*' || tokens[position].value.toChar() == '/')){
        Operation operation;
        if(tokens[position].value.toChar() == '*') {
            operation = Operation::Multiply;
        } else if(tokens[position].value.toChar() == '/') {
            operation = Operation::Divide;
        }

        ++position;
        power();

        operations.push_back(operation);
    }
}

void ExpressionParser::power()
{
    primary();
    CHECK_ERROR

    if(position >= tokens.size()) {
        return;
    }

    while(position < tokens.size() && tokens[position].value.toChar() == '^'){
        Operation operation = Operation::Power;
        ++position;
        primary();

        operations.push_back(operation);
    }
}

void ExpressionParser::primary()
{
    if(position >= tokens.size()) {
        error();
        return;
    }
    if(tokens[position].type == TokenType::Operation && tokens[position].value.toChar() == '-') {
        ++position;
        expression();
        CHECK_ERROR
        operations.push_back(Operation::Neg);
    } else if(tokens[position].type == TokenType::Const) {
        constants.push_back(tokens[position].value.toDouble());
        operations.push_back(Operation::Ldconst);
        ++position;
    } else if(tokens[position].type == TokenType::Id) {
        if(tokens[position].value.toString() == "x") {
            operations.push_back(Operation::Ldx);
            ++position;
        } else if(tokens[position].value.toString() == "sin") {
            ++position;
            if(position >= tokens.size()) {
                error();
            }
            expression();
            CHECK_ERROR
            operations.push_back(Operation::Sin);
        } else if(tokens[position].value.toString() == "cos") {
            ++position;
            if(position >= tokens.size()) {
                error();
            }
            expression();
            CHECK_ERROR
            operations.push_back(Operation::Cos);
        } else {
            error();
        }
    } else {
        error();
    }
}

void ExpressionParser::error()
{
    hasError = true;
}

double Expression::evaluate(double x)
{
    QStack<double> stack;
    int constIdx = 0;
    for(int i = 0; i < operations.size(); ++i) {
        switch(operations[i]) {
            case Operation::Plus:
            case Operation::Minus:
            case Operation::Multiply:
            case Operation::Divide:
            case Operation::Power: {
                auto right = stack.pop();
                auto left = stack.pop();
                switch(operations[i]) {
                case Operation::Plus:
                    stack.push(left + right);
                    break;
                case Operation::Minus:
                    stack.push(left - right);
                    break;
                case Operation::Multiply:
                    stack.push(left * right);
                    break;
                case Operation::Divide:
                    stack.push(left / right);
                    break;
                case Operation::Power:
                    stack.push(pow(left, right));
                    break;
                }

                break;
            }
            case Operation::Ldconst:
                stack.push(constants[constIdx++]);
                break;
            case Operation::Ldx:
                stack.push(x);
                break;
            case Operation::Neg:
                stack.push(-stack.pop());
                break;
            case Operation::Sin:
                stack.push(sin(stack.pop()));
                break;
            case Operation::Cos:
                stack.push(cos(stack.pop()));
                break;

        }
    }

    if(stack.size() != 1) {
        printf("Stack size larger than expected");
    }

    return stack.pop();
}

#undef CHECK_ERROR
