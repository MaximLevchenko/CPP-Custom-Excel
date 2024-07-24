#include "CustomExpressionBuilder.h"

// Implementation for CustomExpressionBuilder methods

void CustomExpressionBuilder::opAdd() {
    expression.push(std::make_unique<BinaryOperation>("+"));
}

void CustomExpressionBuilder::opSub() {
    expression.push(std::make_unique<BinaryOperation>("-"));
}

void CustomExpressionBuilder::opMul() {
    expression.push(std::make_unique<BinaryOperation>("*"));
}

void CustomExpressionBuilder::opDiv() {
    expression.push(std::make_unique<BinaryOperation>("/"));
}

void CustomExpressionBuilder::opPow() {
    expression.push(std::make_unique<BinaryOperation>("^"));
}

void CustomExpressionBuilder::opNeg() {
    expression.push(std::make_unique<UnaryOperation>("-"));
}

void CustomExpressionBuilder::opEq() {
    expression.push(std::make_unique<BinaryOperation>("="));
}

void CustomExpressionBuilder::opNe() {
    expression.push(std::make_unique<BinaryOperation>("<>"));
}

void CustomExpressionBuilder::opLt() {
    expression.push(std::make_unique<BinaryOperation>("<"));
}

void CustomExpressionBuilder::opLe() {
    expression.push(std::make_unique<BinaryOperation>("<="));
}

void CustomExpressionBuilder::opGt() {
    expression.push(std::make_unique<BinaryOperation>(">"));
}

void CustomExpressionBuilder::opGe() {
    expression.push(std::make_unique<BinaryOperation>(">="));
}

void CustomExpressionBuilder::valNumber(double val) {
    expression.push(std::make_unique<Constant>(val));
}

void CustomExpressionBuilder::valString(std::string val) {
    expression.push(std::make_unique<StringVariable>(std::move(val)));
}

void CustomExpressionBuilder::valReference(std::string val) {
    expression.push(std::make_unique<Reference>(std::move(val)));
}

void CustomExpressionBuilder::valRange(std::string val) {
    expression.push(std::make_unique<Range>(std::move(val)));
}

void CustomExpressionBuilder::funcCall(std::string fnName, int paramCount) {
    expression.push(std::make_unique<FunctionCall>(std::move(fnName), paramCount));
}

const std::stack<std::shared_ptr<ExprElement>> &CustomExpressionBuilder::getExpression() const {
    return expression;
}
