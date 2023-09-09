#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <QString>
#include <QQueue>
#include <QVector>
#include <QVariant>

enum class Operation {
    Ldconst,
    Ldx,
    Plus,
    Minus,
    Multiply,
    Divide,
    Power,
    Neg,
    Sin,
    Cos
};

enum class TokenType {
    Id,
    Const,
    Operation,
    Lpar,
    Rpar
};

class Token {
public:
    TokenType type;
    QVariant value;
};

class Expression {
public:
    static Expression invalid;
    double evaluate(double x);
    bool isValid() { return _isValid; }
    explicit Expression(const QVector<double>& constants, const QVector<Operation>& operations): constants(constants), operations(operations), _isValid(true) { }

private:
    QVector<double> constants;
    QVector<Operation> operations;
    bool _isValid;
    explicit Expression(bool valid): _isValid(valid) { }
};

class ExpressionParser
{
public:
    ExpressionParser();

    Expression parse(const QString& str);

private:
    void lex(const QString& str);
    Expression parse();
    void expression();
    void term();
    void factor();
    void power();
    void primary();
    void error();

    int position = 0;
    bool hasError = false;
    QVector<Token> tokens;
    QVector<double> constants;
    QVector<Operation> operations;
};

#endif // EXPRESSIONPARSER_H
