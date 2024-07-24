#include "ExprElement.h"

// Evaluates a stack of expression elements
static CValue evaluateExpression(const std::stack<std::shared_ptr<ExprElement>> &exprStack,
                                 const std::unordered_map<size_t, CustomCValue> &sheet,
                                 std::unordered_set<size_t> &evaluationPath) {
    // Reverse the stack for correct evaluation order
    std::stack<std::shared_ptr<ExprElement>> copyExprStack;
    auto tempStack = exprStack;
    while (!tempStack.empty()) {
        copyExprStack.push(tempStack.top());
        tempStack.pop();
    }

    std::stack<CValue> evalStack;
    while (!copyExprStack.empty()) {
        auto element = copyExprStack.top();
        copyExprStack.pop();
        try {
            element->evaluate(evalStack, sheet, evaluationPath);
        } catch (const std::exception &e) {
            return CValue(); // Return error value on exception
        }
    }

    if (evalStack.size() != 1) {
        throw std::runtime_error("Invalid expression: more than one value remains after evaluation");
    }

    return evalStack.top();
}

// Implementation for Constant class
Constant::Constant(double val) : value(val) {}

void Constant::evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                        std::unordered_set<size_t> &evaluationPath) const {
    evalStack.push(value);
}

std::string Constant::save() const {
    return "Constant " + std::to_string(value);
}

// Implementation for StringVariable class
StringVariable::StringVariable(std::string name) : name(std::move(name)) {}

void StringVariable::evaluate(std::stack<CValue> &evalStack,
                              const std::unordered_map<size_t, CustomCValue> &sheet,
                              std::unordered_set<size_t> &evaluationPath) const {
    evalStack.push(name);
}

std::string StringVariable::save() const {
    std::string escapedName;
    for (char ch : name) {
        if (ch == '"') {
            escapedName += "\"\"";  // Escape quotes
        } else {
            escapedName += ch;
        }
    }
    return "String \"" + escapedName + "\"";
}

// Implementation for BinaryOperation class
BinaryOperation::BinaryOperation(std::string op) : op(std::move(op)) {}

std::string BinaryOperation::getOp() const { return op; }

std::string BinaryOperation::save() const {
    return "BinaryOperation " + op;
}

void BinaryOperation::evaluate(std::stack<CValue> &evalStack,
                               const std::unordered_map<size_t, CustomCValue> & /*sheet*/,
                               std::unordered_set<size_t> &evaluationPath) const {
    if (evalStack.size() < 2) {
        throw std::runtime_error("Insufficient operands for binary operation");
    }

    auto rightValue = evalStack.top();
    evalStack.pop();
    auto leftValue = evalStack.top();
    evalStack.pop();

    CValue result = perform(op, leftValue, rightValue);
    if (std::holds_alternative<std::monostate>(result)) {
        throw std::runtime_error("Invalid operation or operand types");
    }
    evalStack.push(result);
}

CValue BinaryOperation::perform(const std::string &op, const CValue &left, const CValue &right) const {
    if (op == "+") {
        if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
            std::string leftStr = std::holds_alternative<std::string>(left) ? std::get<std::string>(left)
                                                                            : std::to_string(std::get<double>(left));
            std::string rightStr = std::holds_alternative<std::string>(right) ? std::get<std::string>(right)
                                                                              : std::to_string(std::get<double>(right));
            return leftStr + rightStr;
        } else if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            return std::get<double>(left) + std::get<double>(right);
        }
    } else if (op == "-" || op == "*" || op == "/" || op == "^") {
        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            double leftNum = std::get<double>(left);
            double rightNum = std::get<double>(right);
            if (op == "-") return leftNum - rightNum;
            else if (op == "*") return leftNum * rightNum;
            else if (op == "/") return (rightNum == 0) ? CValue(std::monostate()) : CValue(leftNum / rightNum);
            else if (op == "^") return std::pow(leftNum, rightNum);
            else return std::monostate();
        }
    } else if (op == "<" || op == "<=" || op == ">" || op == ">=" || op == "=" || op == "<>") {
        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            double leftNum = std::get<double>(left);
            double rightNum = std::get<double>(right);
            if (op == "<") return leftNum < rightNum ? 1.0 : 0.0;
            else if (op == "<=") return leftNum <= rightNum ? 1.0 : 0.0;
            else if (op == ">") return leftNum > rightNum ? 1.0 : 0.0;
            else if (op == ">=") return leftNum >= rightNum ? 1.0 : 0.0;
            else if (op == "=") return leftNum == rightNum ? 1.0 : 0.0;
            else if (op == "<>") return leftNum != rightNum ? 1.0 : 0.0;
        } else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
            std::string leftStr = std::get<std::string>(left);
            std::string rightStr = std::get<std::string>(right);
            if (op == "=") return leftStr == rightStr ? 1.0 : 0.0;
            else if (op == "<>") return leftStr != rightStr ? 1.0 : 0.0;
            else if (op == "<") return leftStr < rightStr ? 1.0 : 0.0;
            else if (op == "<=") return leftStr <= rightStr ? 1.0 : 0.0;
            else if (op == ">") return leftStr > rightStr ? 1.0 : 0.0;
            else if (op == ">=") return leftStr >= rightStr ? 1.0 : 0.0;
        }
    }
    return std::monostate();
}

// Implementation for UnaryOperation class
UnaryOperation::UnaryOperation(std::string op) : op(std::move(op)) {}

std::string UnaryOperation::getOp() const { return op; }

std::string UnaryOperation::save() const {
    return "UnaryOperation " + op;
}

void UnaryOperation::evaluate(std::stack<CValue> &evalStack,
                              const std::unordered_map<size_t, CustomCValue> &sheet,
                              std::unordered_set<size_t> &evaluationPath) const {
    if (evalStack.empty()) {
        throw std::runtime_error("No operand for unary operation");
    }
    CValue topValue = evalStack.top();
    evalStack.pop();
    double operand;
    try {
        operand = std::get<double>(topValue);
    } catch (const std::bad_variant_access &) {
        throw std::runtime_error("Operand for unary operation is not a number.");
    }
    evalStack.push(apply(op, operand));
}

double UnaryOperation::apply(const std::string &op, double value) const {
    if (op == "-") return -value;
    else throw std::invalid_argument("Unsupported unary operation");
}

// Implementation for Range class
Range::Range(std::string rangeRef) : rangeRef(std::move(rangeRef)) {}

void Range::evaluate(std::stack<CValue> &evalStack,
                     const std::unordered_map<size_t, CustomCValue> &sheet,
                     std::unordered_set<size_t> &evaluationPath) const {
    evalStack.push(rangeRef);
}

std::string Range::save() const {
    return "Range " + rangeRef;
}

// Implementation for FunctionCall class
FunctionCall::FunctionCall(std::string fnName, size_t paramCount)
        : functionName(std::move(fnName)), parameterCount(paramCount) {}

std::string FunctionCall::save() const {
    return "Function " + functionName + " " + std::to_string(parameterCount);
}

void FunctionCall::evaluate(std::stack<CValue> &evalStack,
                            const std::unordered_map<size_t, CustomCValue> &sheet,
                            std::unordered_set<size_t> &evaluationPath) const {
    if (evalStack.size() < parameterCount) {
        throw std::runtime_error("Not enough parameters for function call");
    }

    std::vector<CValue> params;
    for (size_t i = 0; i < parameterCount; ++i) {
        params.push_back(evalStack.top());
        evalStack.pop();
    }
    std::reverse(params.begin(), params.end());

    if (functionName != "countval" && functionName != "if" && !std::holds_alternative<std::string>(params[0])) {
        throw std::runtime_error(functionName + " function expects a range parameter");
    }
    CPos start(0, 0);
    CPos end(0, 0);

    auto removeDollarInRange = [](const std::string &ref) -> std::string {
        std::string cleanedRef;
        std::copy_if(ref.begin(), ref.end(), std::back_inserter(cleanedRef), [](char c) { return c != '$'; });
        return cleanedRef;
    };

    if (functionName != "if") {
        std::string rangeStr = (functionName != "countval" ? std::get<std::string>(params[0])
                                                           : std::get<std::string>(params[1]));
        auto posDelimiter = rangeStr.find(':');
        if (posDelimiter == std::string::npos) {
            throw std::runtime_error("Invalid range format");
        }

        std::string startStr = removeDollarInRange(rangeStr.substr(0, posDelimiter));
        std::string endStr = removeDollarInRange(rangeStr.substr(posDelimiter + 1));

        start = CPos(startStr);
        end = CPos(endStr);
    }
    if (functionName == "sum") {
        double sum = 0;
        bool hasNumeric = false;
        for (size_t r = start.getRow(); r <= end.getRow(); ++r) {
            for (size_t c = start.getColumn(); c <= end.getColumn(); ++c) {
                CPos pos(c, r);
                auto it = sheet.find(pos.getUniqueId());
                if (it != sheet.end() && std::holds_alternative<double>(it->second)) {
                    sum += std::get<double>(it->second);
                    hasNumeric = true;
                } else if (it != sheet.end() &&
                           std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                    const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                    CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                    if (std::holds_alternative<double>(result)) {
                        sum += std::get<double>(result);
                        hasNumeric = true;
                    }
                }
            }
        }
        if (!hasNumeric) {
            throw std::runtime_error("No numeric values found in the range for sum computation");
        }
        evalStack.push(sum);
    } else if (functionName == "count") {
        size_t count = 0;
        for (size_t r = start.getRow(); r <= end.getRow(); ++r) {
            for (size_t c = start.getColumn(); c <= end.getColumn(); ++c) {
                CPos pos(c, r);
                auto it = sheet.find(pos.getUniqueId());
                if (it != sheet.end() && !std::holds_alternative<std::monostate>(it->second)) {
                    if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                        const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                        CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                        if (!std::holds_alternative<std::monostate>(result))
                            count++;
                    } else
                        count++;
                }
            }
        }
        evalStack.push(static_cast<double>(count));
    } else if (functionName == "min") {
        std::optional<double> minVal;
        for (size_t r = start.getRow(); r <= end.getRow(); ++r) {
            for (size_t c = start.getColumn(); c <= end.getColumn(); ++c) {
                CPos pos(c, r);
                auto it = sheet.find(pos.getUniqueId());
                if (it != sheet.end() && std::holds_alternative<double>(it->second)) {
                    double val = std::get<double>(it->second);
                    if (!minVal || val < *minVal) {
                        minVal = val;
                    }
                } else if (it != sheet.end() &&
                           std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                    const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                    CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                    if (std::holds_alternative<double>(result)) {
                        double val = std::get<double>(result);
                        if (!minVal || val < *minVal) {
                            minVal = val;
                        }
                    }
                }
            }
        }
        if (minVal) {
            evalStack.push(*minVal);
        } else {
            throw std::runtime_error("No numeric values found for min function");
        }
    } else if (functionName == "max") {
        std::optional<double> maxVal;
        for (size_t r = start.getRow(); r <= end.getRow(); ++r) {
            for (size_t c = start.getColumn(); c <= end.getColumn(); ++c) {
                CPos pos(c, r);
                auto it = sheet.find(pos.getUniqueId());
                if (it != sheet.end() && std::holds_alternative<double>(it->second)) {
                    double val = std::get<double>(it->second);
                    if (!maxVal || val > *maxVal) {
                        maxVal = val;
                    }
                } else if (it != sheet.end() &&
                           std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                    const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                    CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                    if (std::holds_alternative<double>(result)) {
                        double val = std::get<double>(result);
                        if (!maxVal || val > *maxVal) {
                            maxVal = val;
                        }
                    }
                }
            }
        }
        if (maxVal) {
            evalStack.push(*maxVal);
        } else {
            throw std::runtime_error("No numeric values found for max function");
        }
    } else if (functionName == "countval") {
        if (parameterCount != 2) {
            throw std::runtime_error("countval expects two parameters");
        }
        CValue valueToMatch = params[0];
        size_t count = 0;
        for (size_t r = start.getRow(); r <= end.getRow(); ++r) {
            for (size_t c = start.getColumn(); c <= end.getColumn(); ++c) {
                CPos pos(c, r);
                auto it = sheet.find(pos.getUniqueId());
                if (it != sheet.end()) {
                    const auto &cellValue = it->second;
                    if (std::holds_alternative<double>(cellValue) && std::holds_alternative<double>(valueToMatch)) {
                        count += (std::get<double>(cellValue) == std::get<double>(valueToMatch));
                    } else if (std::holds_alternative<std::string>(cellValue) &&
                               std::holds_alternative<std::string>(valueToMatch)) {
                        count += (std::get<std::string>(cellValue) == std::get<std::string>(valueToMatch));
                    } else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(cellValue)) {
                        const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(cellValue);
                        CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                        if (std::holds_alternative<double>(result) &&
                            std::holds_alternative<double>(valueToMatch)) {
                            count += (std::get<double>(result) == std::get<double>(valueToMatch));
                        } else if (std::holds_alternative<std::string>(result) &&
                                   std::holds_alternative<std::string>(valueToMatch)) {
                            count += (std::get<std::string>(result) == std::get<std::string>(valueToMatch));
                        }
                    }
                }
            }
        }
        evalStack.push(static_cast<double>(count));
    } else if (functionName == "if") {
        if (parameterCount != 3) {
            throw std::runtime_error("Invalid parameter count for if function");
        }
        bool condition = false;
        if (auto cond = std::get_if<double>(&params[0])) {
            condition = *cond != 0.0;
        } else {
            throw std::runtime_error("Conditional expression in 'if' did not evaluate to a numeric type.");
        }
        evalStack.push(condition ? params[1] : params[2]);
    } else {
        throw std::runtime_error("Unknown function call");
    }
}

// Implementation for Reference class
Reference::Reference(std::string cellRef) : cellReference(std::move(cellRef)) {
    parseReference(cellReference);
}

std::string Reference::save() const {
    return "Reference " + cellReference;
}

void Reference::evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                         std::unordered_set<size_t> &evaluationPath) const {
    CPos position(column, row);
    auto it = sheet.find(position.getUniqueId());
    if (it == sheet.end()) {
        throw std::runtime_error("Reference not found in spreadsheet context.");
    }

    const auto &value = it->second;
    if (std::holds_alternative<double>(value)) {
        evalStack.push(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        evalStack.push(std::get<std::string>(value));
    } else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(value)) {
        if (!evaluationPath.insert(position.getUniqueId()).second) {
            evaluationPath.clear();
            throw std::runtime_error("Cyclic dependency detected!");
        }
        const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(value);
        CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
        evaluationPath.erase(position.getUniqueId());
        evalStack.push(result);
    } else {
        throw std::runtime_error("Unexpected cell content encountered during evaluation.");
    }
}

void Reference::moveRelativeReferencesBy(const CPos &offset) {
    if (!isAbsoluteRow) {
        row += offset.getRow();
    }
    if (!isAbsoluteColumn) {
        column += offset.getColumn();
    }
    updateCellReferenceString();
}

void Reference::parseReference(const std::string &ref) {
    size_t i = 0;
    column = 0;
    row = 0;
    isAbsoluteColumn = false;
    isAbsoluteRow = false;
    if (ref[i] == '$') {
        isAbsoluteColumn = true;
        i++;
    }
    while (i < ref.size() && std::isalpha(ref[i])) {
        column = column * 26 + (toupper(ref[i]) - 'A' + 1);
        i++;
    }
    if (i < ref.size() && ref[i] == '$') {
        isAbsoluteRow = true;
        i++;
    }
    if (i < ref.size()) {
        row = std::stoul(ref.substr(i));
    } else {
        throw std::invalid_argument("Invalid cell reference format");
    }
}

void Reference::updateCellReferenceString() {
    cellReference.clear();
    if (isAbsoluteColumn) {
        cellReference += '$';
    }
    size_t tempColumn = column;
    std::string columnPart;
    while (tempColumn > 0) {
        char letter = 'A' + (tempColumn - 1) % 26;
        columnPart = letter + columnPart;
        tempColumn = (tempColumn - 1) / 26;
    }
    cellReference += columnPart;
    if (isAbsoluteRow) {
        cellReference += '$';
    }
    cellReference += std::to_string(row);
}
