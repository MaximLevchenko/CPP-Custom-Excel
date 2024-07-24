#ifndef __PROGTEST__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <utility>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"

using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;
#endif /* __PROGTEST__ */

class CPos {
public:
    // Constructor from a string.
    CPos(std::string_view str) {
        parseCellRef(str);
        updateUniqueId();
    }
    // Constructor from column and row numbers directly.
    CPos(size_t col, size_t row) : column(col), row(row) {
        updateUniqueId();
    }
    // Subtract operator to calculate the offset.
    CPos operator-(const CPos &other) const {
        return CPos(column - other.column, row - other.row);
    }
    // Shift method to return a new CPos shifted by dx and dy.
    CPos shift(int dx, int dy) const {
        size_t newColumn = column + dx;
        size_t newRow = row + dy;
        return CPos(newColumn, newRow);
    }
    size_t getUniqueId() const {
        return uniqueId;
    }
    // Getters for column and row.
    size_t getColumn() const { return column; }
    size_t getRow() const { return row; }
private:
    size_t column, row;
    size_t uniqueId;

    void updateUniqueId() {
        uniqueId = (column << (sizeof(size_t) * 4)) + row;
    }
    void parseCellRef(std::string_view cellRef) {
        size_t i = 0;
        column = 0;
        // Process column characters (base-26 conversion).
        while (i < cellRef.size() && std::isalpha(cellRef[i])) {
            column = column * 26 + (toupper(cellRef[i]) - 'A' + 1);
            ++i;
        }
        // Ensure we have row numbers after the column letters.
        if (i < cellRef.size()) {
            row = std::stoull(std::string(cellRef.substr(i)));
        } else {
            throw std::invalid_argument("Invalid cell reference format");
        }
    }
};

// Forward declare ExprElement
class ExprElement;

using CustomCValue = std::variant<std::monostate, double, std::string, int, std::stack<std::shared_ptr<ExprElement>>>;

class ExprElement {
public:
    virtual ~ExprElement() = default;
    virtual void
    evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
             std::unordered_set<size_t> &evaluationPath) const = 0;
    virtual std::string save() const = 0;
};

class Constant : public ExprElement {
    double value;
public:
    explicit Constant(double val) : value(val) {}
    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override {
        evalStack.push(value);
    }
    std::string save() const override {
        return "Constant " + std::to_string(value);
    }
};

class StringVariable : public ExprElement {
    std::string name;
public:
    explicit StringVariable(std::string name) : name(std::move(name)) {}
    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override {
        evalStack.push(name);
    }
    std::string save() const override {
        std::string escapedName;
        for (char ch: name) {
            if (ch == '"') {
                escapedName += "\"\"";  // Double the quotes for escaping
            } else {
                escapedName += ch;
            }
        }
        return "String \"" + escapedName + "\"";
    }
};

//todo make it so operands can be operated with string + cislo vice versa string + string
class BinaryOperation : public ExprElement {
    std::string op; // Operator is now a char, not a string.
public:
    explicit BinaryOperation(std::string op) : op(std::move(op)) {}
    std::string getOp() const { return op; }
    std::string save() const override {
        return "BinaryOperation " + op;
    }
    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> & /*sheet*/,
                  std::unordered_set<size_t> &evaluationPath) const override {
        if (evalStack.size() < 2) {
            throw std::runtime_error("Insufficient operands for binary operation");
        }

        auto rightValue = evalStack.top();
        evalStack.pop();
        auto leftValue = evalStack.top();
        evalStack.pop();

        // Process operands based on operation rules
        CValue result = perform(op, leftValue, rightValue);
        //check for monostate, because we want to return empty in evaluateExpression
        if (std::holds_alternative<std::monostate>(result)) {
            throw std::runtime_error("Invalid operation or operand types");
        }
        evalStack.push(result);
    }

    CValue perform(const std::string &op, const CValue &left, const CValue &right) const {
        if (op == "+") {
            if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
                std::string leftStr = std::holds_alternative<std::string>(left) ? std::get<std::string>(left)
                                                                                : std::to_string(
                                std::get<double>(left));
                std::string rightStr = std::holds_alternative<std::string>(right) ? std::get<std::string>(right)
                                                                                  : std::to_string(
                                std::get<double>(right));
                return leftStr + rightStr;
            } else if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            }
            //fall to monostate
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
            //fall to monostate
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
            //fall to monostate
        }
        return std::monostate(); // Return undefined value if operands or operations do not match rules
    }
};

class UnaryOperation : public ExprElement {
    std::string op;
public:
    explicit UnaryOperation(std::string op) : op(std::move(op)) {}
    std::string getOp() const { return op; }
    std::string save() const override {
        return "UnaryOperation " + op;
    }
    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> & /*sheet*/,
                  std::unordered_set<size_t> &evaluationPath) const override {
        if (evalStack.empty()) {
            throw std::runtime_error("No operand for unary operation");
        }
        CValue topValue = evalStack.top();
        evalStack.pop();
        double operand;
        try {
            operand = std::get<double>(topValue); // Attempt to get operand as double
        } catch (const std::bad_variant_access &) {
            throw std::runtime_error("Operand for unary operation is not a number.");
        }
        evalStack.push(apply(op, operand));
    }

    double apply(const std::string &op, double value) const {
        if (op == "-") return -value;
        else throw std::invalid_argument("Unsupported unary operation");
    }
};

class Range : public ExprElement {
    std::string rangeRef; // Example: "A1:B2"

public:
    explicit Range(std::string rangeRef) : rangeRef(std::move(rangeRef)) {}

    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override {
        evalStack.push(rangeRef);
    }
    std::string save() const override {
        return "Range " + rangeRef;
    }
};

static CValue evaluateExpression(const std::stack<std::shared_ptr<ExprElement>> &exprStack,
                                 const std::unordered_map<size_t, CustomCValue> &sheet,
                                 std::unordered_set<size_t> &evaluationPath) {
    // Reverse the stack to get the right order for evaluation.
    std::stack<std::shared_ptr<ExprElement>> copyExprStack;
    auto tempStack = exprStack; // Copy because we cannot mutate the original stack directly.
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
            // Handle evaluation error
            return CValue(); // or some error value
        }
        // Call the evaluate method of the ExprElement, which knows how to evaluate itself.
    }

    if (evalStack.size() != 1) {
        throw std::runtime_error("Invalid expression: more than one value remains after evaluation");
    }

    return evalStack.top();
}

class FunctionCall : public ExprElement {
    std::string functionName;
    size_t parameterCount;

public:
    FunctionCall(std::string fnName, size_t paramCount)
            : functionName(std::move(fnName)), parameterCount(paramCount) {}
    std::string save() const override {
        return "Function " + functionName + " " + std::to_string(parameterCount);
    }
    void evaluate(std::stack<CValue> &evalStack,
                  const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override {
        // Check if there are enough parameters in the stack
        if (evalStack.size() < parameterCount) {
            throw std::runtime_error("Not enough parameters for function call");
        }

        // Retrieve parameters from the stack
        std::vector<CValue> params;
        for (size_t i = 0; i < parameterCount; ++i) {
            params.push_back(evalStack.top());
            evalStack.pop();
        }
        //maybe instead of doing reverse everywhere, you just reverse it one time in the CExpressionBuilder and thats it
        //we should reverse to save the order
        //2*8 would be parsed as 2 8 *, then once again onto the evalstack it would be 8 2 *, so we reverse to preserve order
        std::reverse(params.begin(), params.end());

        // First parameter is expected to be the range for all functions except countval
        if (functionName != "countval" && functionName != "if" && !std::holds_alternative<std::string>(params[0])) {
            throw std::runtime_error(functionName + " function expects a range parameter");
        }
        CPos start(0, 0);//placeholder maybe if we have if
        CPos end(0, 0);
        // Function to remove dollar signs from cell references. We dont need dollar signs here at all
        auto removeDollarInRange = [](const std::string &ref) -> std::string {

            std::string cleanedRef;
            std::copy_if(ref.begin(), ref.end(), std::back_inserter(cleanedRef),
                         [](char c) { return c != '$'; });
            return cleanedRef;
        };

        // Parsing the range
        if (functionName != "if") {
            std::string rangeStr = (functionName != "countval" ? std::get<std::string>(params[0])
                                                               : std::get<std::string>(params[1]));
            auto posDelimiter = rangeStr.find(':');
            if (posDelimiter == std::string::npos) {
                throw std::runtime_error("Invalid range format");
            }

            std::string startStr = removeDollarInRange(rangeStr.substr(0, posDelimiter)); //dont need dollar signs
            std::string endStr = removeDollarInRange(rangeStr.substr(posDelimiter + 1)); //dont need dollar signs

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
                    }
                        //it might also be an expression
                    else if (it != sheet.end() &&
                             std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                        const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                        CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                        //then if its a number we add
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
                    //if its an expression
                    if (it != sheet.end() && !std::holds_alternative<std::monostate>(it->second)) {
                        //expression
                        if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                            const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                            CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                            if (!std::holds_alternative<std::monostate>(result))
                                count++;
                        } else //not monostate and not expression, just add count
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
            //true if contains value, false otherwise
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
                            if (!maxVal || val < *maxVal) {
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
                        //two doubles
                        if (std::holds_alternative<double>(cellValue) && std::holds_alternative<double>(valueToMatch)) {
                            count += (std::get<double>(cellValue) == std::get<double>(valueToMatch));
                        }
                            //two strings
                        else if (std::holds_alternative<std::string>(cellValue) &&
                                 std::holds_alternative<std::string>(valueToMatch)) {
                            count += (std::get<std::string>(cellValue) == std::get<std::string>(valueToMatch));
                        }
                            //if cell value has expression, valueToMatch cannot have expression at this stage
                        else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(cellValue)) {
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
            // params[0] is the condition, params[1] is the true case, and params[2] is the false case.
            bool condition = false;
            if (auto cond = std::get_if<double>(&params[0])) {
                condition = *cond != 0.0;
            } else {
                throw std::runtime_error("Conditional expression in 'if' did not evaluate to a numeric type.");
            }
// at this point parameters can only be CValue, cause if they were an expression, it would have been transformed to CValue
            evalStack.push(condition ? params[1] : params[2]);
        } else {
            throw std::runtime_error("Unknown function call");
        }
    }
};

class Reference : public ExprElement {
    size_t row, column;
    bool isAbsoluteRow, isAbsoluteColumn;
    std::string cellReference;

public:
    explicit Reference(std::string cellRef) : cellReference(std::move(cellRef)) {
        parseReference(cellReference);
    }
    std::string save() const override {
        return "Reference " + cellReference;
    }

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override {
        // Construct a CPos object from the row and column. This assumes CPos can be constructed this way.
        CPos position(column, row);
        auto it = sheet.find(position.getUniqueId());
        if (it == sheet.end()) {
            throw std::runtime_error("Reference not found in spreadsheet context.");
        }

        //we know its not end so we do not check for it here
        const auto &value = it->second;
        if (std::holds_alternative<double>(value)) {
            evalStack.push(std::get<double>(value));
        } else if (std::holds_alternative<std::string>(value)) {
            evalStack.push(std::get<std::string>(value));
        } else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(value)) {
            if (!evaluationPath.insert(position.getUniqueId()).second) {
                // Cycle detected
                evaluationPath.clear(); // Clear the path after detection
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

    // Method to move relative references within the Reference by a given offset.
    void moveRelativeReferencesBy(const CPos &offset) {
        if (!isAbsoluteRow) {
            row += offset.getRow();
        }
        if (!isAbsoluteColumn) {
            column += offset.getColumn();
        }
        // Rebuild the cellReference string based on current row, column, and absolute flags.
        updateCellReferenceString();
    }

private:
    void parseReference(const std::string &ref) {
        size_t i = 0;
        column = 0;
        row = 0;
        isAbsoluteColumn = false;
        isAbsoluteRow = false;
        // Parse the column part (letters)
        if (ref[i] == '$') {
            isAbsoluteColumn = true;
            i++;
        }
        while (i < ref.size() && std::isalpha(ref[i])) {
            column = column * 26 + (toupper(ref[i]) - 'A' + 1);
            i++;
        }
        // Parse the row part (numbers)
        if (i < ref.size() && ref[i] == '$') {
            isAbsoluteRow = true;
            i++;
        }
        if (i < ref.size()) {
            row = std::stoul(ref.substr(i)); // Converts the rest of the string to a number
        } else {
            throw std::invalid_argument("Invalid cell reference format");
        }
    }

    void updateCellReferenceString() {
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
};

class CustomExpressionBuilder : public CExprBuilder {
public:
    void opAdd() override {
//        std::cout << "+";
        expression.push(std::make_unique<BinaryOperation>("+"));
    }
    void opSub() override {
//        std::cout << "-";
        expression.push(std::make_unique<BinaryOperation>("-"));
    }
    void opMul() override {
//        std::cout << "*";
        expression.push(std::make_unique<BinaryOperation>("*"));
    }
    void opDiv() override {
        expression.push(std::make_unique<BinaryOperation>("/"));
    }
    void opPow() override {
        expression.push(std::make_unique<BinaryOperation>("^"));
    }
    void opNeg() override {
//        std::cout << "-";
        expression.push(std::make_unique<UnaryOperation>("-"));
    }
    void opEq() override {
        expression.push(std::make_unique<BinaryOperation>("="));
    }
    void opNe() override {
        expression.push(std::make_unique<BinaryOperation>("<>"));
    }
    void opLt() override {
        expression.push(std::make_unique<BinaryOperation>("<"));
    }
    void opLe() override {
        expression.push(std::make_unique<BinaryOperation>("<="));
    }
    void opGt() override {
        expression.push(std::make_unique<BinaryOperation>(">"));
    }
    void opGe() override {
        expression.push(std::make_unique<BinaryOperation>(">="));
    }
    void valNumber(double val) override {
//        std::cout << val;
        expression.push(std::make_unique<Constant>(val));
    }
    void valString(std::string val) override {
//        std::cout << val;
        expression.push(std::make_unique<StringVariable>(val));
    }
    void valReference(std::string val) override {
        expression.push(std::make_unique<Reference>(val));
    }
    void valRange(std::string val) override {
//        std::cout << val << " - range\n";
        expression.push(std::make_unique<Range>(val));
    }
    void funcCall(std::string fnName, int paramCount) override {
//        std::cout << fnName << " - fnName, " << paramCount << " - paramCount\n";
        expression.push(std::make_unique<FunctionCall>(fnName, paramCount));
    }
    const std::stack<std::shared_ptr<ExprElement>> &getExpression() const {
        return expression;
    }
private:
    std::stack<std::shared_ptr<ExprElement>> expression;
};

class CSpreadsheet {
public:
    static unsigned capabilities() {
        return SPREADSHEET_CYCLIC_DEPS | SPREADSHEET_FILE_IO | SPREADSHEET_SPEED;
    }
    CSpreadsheet() = default;
    bool load(std::istream &is) {
        std::string line;
        unsigned long calculatedChecksum = 0;
        unsigned long readChecksum;

        // Read the first line for the checksum
        if (!getline(is, line)) return false; // Early return if there's no input
        std::istringstream checksumStream(line);
        std::string checksumLabel;
        checksumStream >> checksumLabel >> readChecksum;
        if (checksumLabel != "CHECKSUM") return false; // Incorrect format if CHECKSUM isn't found

        std::ostringstream contentStream;

        // Process the rest of the lines to calculate checksum
        while (getline(is, line)) {
            contentStream << line << '\n';  // Collect lines to reconstruct the content
            for (char c: line) {
                calculatedChecksum += static_cast<unsigned char>(c); // Ensure positive values for checksum
            }
            calculatedChecksum += '\n'; // Include newline character in checksum calculation as in save
        }

        // Compare the calculated checksum with the read checksum
        if (readChecksum != calculatedChecksum) {
            return false; // Checksum does not match, indicating potential corruption or modification
        }

        // If checksum is valid, parse the content
        std::istringstream dataStream(contentStream.str());
        while (getline(dataStream, line)) {
            std::stringstream ss(line);
            size_t key;
            char delim;
            ss >> key >> delim;  // Read the cell key and the comma delimiter
            ss.get(); // Skip the space after the comma

            if (ss.peek() == '[') {
                // It's an expression stack
                ss.get(); // Remove the '['
                std::stack<std::shared_ptr<ExprElement>> exprStack;
                std::string element;

                while (getline(ss, element, ',')) {
                    // Trim the element to remove leading spaces and the trailing bracket if present
                    element.erase(0, element.find_first_not_of(" "));
                    if (element.back() == ']') {
                        element.erase(element.size() - 1); // Remove the trailing ']'
                    }

                    std::stringstream elemStream(element);
                    std::string type;
                    elemStream >> type; // Extracts the type
                    elemStream >> std::ws; // Consume any whitespace after the type

                    if (type == "Reference") {
                        std::string value;
                        elemStream >> value;
                        exprStack.push(std::make_shared<Reference>(value));
                    } else if (type == "Constant") {
                        double value;
                        elemStream >> value;
                        exprStack.push(std::make_shared<Constant>(value));
                    } else if (type == "UnaryOperation") {
                        std::string op;
                        elemStream >> op;
                        exprStack.push(std::make_shared<UnaryOperation>(op));
                    } else if (type == "BinaryOperation") {
                        std::string op;
                        elemStream >> op;
                        exprStack.push(std::make_shared<BinaryOperation>(op));
                    } else if (type == "String") {
                        std::string content;
                        std::getline(elemStream, content, '"'); // Skip to the first quote
                        std::getline(elemStream, content, '"'); // Read the string content
                        // Handle escaped quotes
                        size_t pos = 0;
                        while ((pos = content.find("\"\"", pos)) != std::string::npos) {
                            content.replace(pos, 2, "\"");
                            pos += 1; // Move past the inserted quote
                        }
                        exprStack.push(std::make_shared<StringVariable>(content));
                    } else if (type == "Range") {
                        std::string range;
                        elemStream >> range;
                        exprStack.push(std::make_shared<Range>(range));
                    } else if (type == "Function") {
                        std::string function_name;
                        double param_count;
                        elemStream >> function_name;
                        elemStream >> std::ws; // Consume any whitespace after the type
                        elemStream >> param_count;
                        exprStack.push(std::make_shared<FunctionCall>(function_name, param_count));
                    }
                }
                sheet[key] = exprStack;  // Assign the constructed stack to the cell
            } else {
                // It's a single value (number or string)
                if (ss.peek() == '"') {
                    // It's a string
                    std::string strValue;
                    getline(ss, strValue, '"');  // Skip the first quote
                    getline(ss, strValue, '"');  // Read the string value
                    // Handle escaped quotes
                    size_t pos = 0;
                    while ((pos = strValue.find("\"\"", pos)) != std::string::npos) {
                        strValue.replace(pos, 2, "\"");
                        pos += 1;
                    }
                    sheet[key] = strValue;
                } else if (ss.peek() != 'u') {
                    // It's a number (and not "undefined")
                    double numValue;
                    ss >> numValue;
                    sheet[key] = numValue;
                } else {
                    // Handle "undefined" or similar cases
                    sheet[key] = std::monostate();
                }
            }
        }
        return true;
    }

    bool save(std::ostream &os) const {
        std::ostringstream contentStream; // To temporarily hold data content
        unsigned long checksum = 0; // Initialize checksum

        for (const auto &[key, val]: sheet) {
            contentStream << key << ", ";  // Map key comes first, followed by ", "
            if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(val)) {
                std::stack<std::shared_ptr<ExprElement>> tempStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(
                        val);
                std::vector<std::shared_ptr<ExprElement>> elements;
                while (!tempStack.empty()) {
                    elements.push_back(tempStack.top());
                    tempStack.pop();
                }
                std::reverse(elements.begin(), elements.end());
                contentStream << "[";
                for (size_t i = 0; i < elements.size(); ++i) {
                    if (i > 0) contentStream << ", ";
                    contentStream << elements[i]->save();
                }
                contentStream << "]";
            } else if (std::holds_alternative<double>(val)) {
                contentStream << std::to_string(std::get<double>(val));
            } else if (std::holds_alternative<std::string>(val)) {
                std::string escapedString;
                for (char ch: std::get<std::string>(val)) {
                    if (ch == '"') {
                        escapedString += "\"\"";  // Double quotes for escaping
                    } else {
                        escapedString += ch;
                    }
                }
                contentStream << '"' << escapedString << '"';
            } else {
                contentStream << "undefined";  // Handle std::monostate or other unexpected types
            }
            contentStream << std::endl;
        }
        // Manually compute a checksum
        std::string data = contentStream.str();
        for (char c: data) {
            checksum += static_cast<unsigned char>(c); // Cast char to unsigned char to ensure positive values
        }

        os << "CHECKSUM " << checksum << std::endl; // Write checksum at the beginning
        os << data; // Write data
        return true;
    }
    bool setCell(const CPos &pos, const std::string &contents) {
        if (!contents.empty() && contents[0] == '=') {
            // It's an expression, we need to parse and evaluate it
            CustomExpressionBuilder exprBuilder;
            parseExpression(contents, exprBuilder); // Parse the expression, omitting the '='
            CustomCValue expression = exprBuilder.getExpression();      // get the expression
            sheet[pos.getUniqueId()] = expression;                 // Store the expression
        } else {
            // Not an expression, handle as before
            CustomCValue value = DetermineValue(contents);
            sheet[pos.getUniqueId()] = value;
        }
        return true;
    }
    CValue getValue(const CPos &pos) const {
        auto it = sheet.find(pos.getUniqueId());
        size_t uniqueId = pos.getUniqueId();
        // Check for cycle

        if (it != sheet.end()) {
            // If the value is not std::monostate, we return the corresponding CValue.
            if (!std::holds_alternative<std::monostate>(it->second)) {
                if (std::holds_alternative<double>(it->second)) {
                    return std::get<double>(it->second);
                } else if (std::holds_alternative<std::string>(it->second)) {
                    return std::get<std::string>(it->second);
                } else if (std::holds_alternative<int>(it->second)) {
                    // Convert int to double for uniformity.
                    return static_cast<double>(std::get<int>(it->second));
                } else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                    if (!evaluationPath.insert(uniqueId).second) {
                        // Cycle detected
                        evaluationPath.clear(); // Clear the path after detection
                        throw std::runtime_error("Cyclic dependency detected!");
                    }
                    // Evaluate the expression and return its value
                    try {
                        const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                        CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                        evaluationPath.clear(); // Clear the path after detection
                        return result;
                    } catch (const std::exception &e) {
                        evaluationPath.clear(); // Clear the path after detection
                        // Handle evaluation error (e.g., variable not found in context)
                        return CValue(); // or some error value
                    }
                }
            }
        }
        evaluationPath.clear(); // Clear the path after detection
        // If the cell does not exist or the value is std::monostate, we return an empty CValue.
        return CValue();
    }
    void copyRect(CPos dst, CPos src, int w = 1, int h = 1) {
        // We need to create a temporary storage to handle overlapping regions to preserve data
        std::vector<std::pair<size_t, CustomCValue>> tempStorage;
        tempStorage.reserve(w * h);

        // Calculate the vector of movement
        auto offset = dst - src;

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                CPos from = src.shift(x, y);
                CPos to = dst.shift(x, y);
                size_t fromId = from.getUniqueId();
                size_t toId = to.getUniqueId();

                auto it = sheet.find(fromId);
                if (it != sheet.end()) {
                    auto &content = it->second;
                    if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(content)) {
                        // If it's an expression, copy and update references
                        std::stack<std::shared_ptr<ExprElement>> exprCopy;//stack for deep copy
                        std::stack<std::shared_ptr<ExprElement>> originalExprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(
                                content);

                        // Reverse the order for iteration
                        std::stack<std::shared_ptr<ExprElement>> reversedCopy;
                        while (!originalExprStack.empty()) {
                            reversedCopy.push(originalExprStack.top());
                            originalExprStack.pop();
                        }

                        // Now create a new stack for the deep copy
                        while (!reversedCopy.empty()) {
                            auto &elem = reversedCopy.top();

                            // Deep copy only if the element is a Reference
                            if (auto ref = dynamic_cast<Reference *>(elem.get())) {
                                // Create a new Reference, using the copy constructor, and update its relative references
                                auto copiedRef = std::make_shared<Reference>(*ref);//deep copy
                                copiedRef->moveRelativeReferencesBy(offset);
                                exprCopy.push(copiedRef);
                            } else {
                                // For other types of elements that don't require updates, just copy the shared_ptr
                                exprCopy.push(elem);
                            }

                            reversedCopy.pop();
                        }

                        tempStorage.emplace_back(toId, std::move(exprCopy));
                    } else {
                        // If it's not an expression (i.e., a string or a number), just copy the value
                        tempStorage.emplace_back(toId, content);
                    }
                } else {
                    // If there's no content in the source cell, ensure the destination cell is cleared.
                    tempStorage.emplace_back(toId, CustomCValue());
                }
            }
        }

        // Now we apply the changes from the temporary storage to the actual spreadsheet.
        for (auto &[id, val]: tempStorage) {
            sheet[id] = std::move(val);
        }
    }
private:
    CustomCValue DetermineValue(const std::string &contents) {
        // Handle empty string
        if (contents.empty()) {
            return std::monostate(); // Represents an empty / undefined CValue
        }
        // Handle string that can be converted to a number
        try {
            size_t idx;
            double numVal = std::stod(contents, &idx);
            if (idx == contents.size()) {
                return numVal; // The whole string is a number
            }
        } catch (const std::invalid_argument &) {
            // Not a number, continue to handle as a string
        } catch (const std::out_of_range &) {
            // Number out of range for a double, handle as a string
        }

        // Handle non-empty string that is not a valid number
        return contents;
    }
    //todo maybe change to uint64_t
    std::unordered_map<size_t, CustomCValue> sheet;
    mutable std::unordered_set<size_t> evaluationPath;
};

#ifndef __PROGTEST__

bool valueMatch(const CValue &r,
                const CValue &s) {
    if (r.index() != s.index())
        return false;
    if (r.index() == 0)
        return true;
    if (r.index() == 2)
        return std::get<std::string>(r) == std::get<std::string>(s);
    if (std::isnan(std::get<double>(r)) && std::isnan(std::get<double>(s)))
        return true;
    if (std::isinf(std::get<double>(r)) && std::isinf(std::get<double>(s)))
        return (std::get<double>(r) < 0 && std::get<double>(s) < 0)
               || (std::get<double>(r) > 0 && std::get<double>(s) > 0);
    return fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
}
int main() {
    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;
//------------ moje testy
//    x0.setCell(CPos("A1"), "=2 8 +");
//    x0.setCell(CPos("AA1"), "=A1+\"sp,f\"");
//    oss.clear();
//    oss.str("");
//    assert ( x0 . save ( oss ) );
//    data = oss . str ();
//    std::cout<<"\n--------------------------------\n";
//    std::cout<<data;
//    iss . clear ();
//    iss . str ( data );
//    assert ( x1 . load ( iss ) );

//    CValue test = x0.getValue(CPos("A1"));
//    CValue test2 = x0.getValue(CPos("AA1"));

//    x0.setCell(CPos("A1"), "sdg");
//    x0.setCell(CPos("AA1"), "sgf+2");
//    CValue test1 =x0.getValue(CPos("AA1"));

//    assert (x0.setCell(CPos("A2"), "=A1"));
//    assert (x0.setCell(CPos("A1"), "=A2"));
//    x0.getValue(CPos("A1"));
//    assert (x0.setCell(CPos("A1"), "10"));
//    x0.setCell(CPos("B3"), "=countval(10, A1:B2)");
//    assert (valueMatch(x0.getValue(CPos("B3")), CValue(1.0)));
//    x0.setCell(CPos("B4"), "=sum(A1:B3)");
//    assert (valueMatch(x0.getValue(CPos("B4")), CValue(11.0)));
//    x0.setCell(CPos("B5"), "=if(sum($B$3:$B$4)=12, 1, 0)");
//    assert (valueMatch(x0.getValue(CPos("B5")), CValue(1.0)));
//------------ moje testy




    assert (x0.setCell(CPos("A1"), "10"));
    assert (x0.setCell(CPos("A2"), "20.5"));
    assert (x0.setCell(CPos("A3"), "3e1"));
    assert (x0.setCell(CPos("A4"), "=40"));
    assert (x0.setCell(CPos("A5"), "=5e+1"));
    assert (x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert (x0.setCell(CPos("A7"),
                       "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));
    assert (valueMatch(x0.getValue(CPos("A1")), CValue(10.0)));
    assert (valueMatch(x0.getValue(CPos("A2")), CValue(20.5)));
    assert (valueMatch(x0.getValue(CPos("A3")), CValue(30.0)));
    assert (valueMatch(x0.getValue(CPos("A4")), CValue(40.0)));
    assert (valueMatch(x0.getValue(CPos("A5")), CValue(50.0)));
    assert (valueMatch(x0.getValue(CPos("A6")),
                       CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert (valueMatch(x0.getValue(CPos("A7")),
                       CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
    assert (valueMatch(x0.getValue(CPos("A8")), CValue()));
    assert (valueMatch(x0.getValue(CPos("AAAA9999")), CValue()));
    assert (x0.setCell(CPos("B1"), "=A1+A2*A3"));
    assert (x0.setCell(CPos("B2"), "= -A1 ^ 2 - A2 / 2   "));
    assert (x0.setCell(CPos("B3"), "= 2 ^ $A$1"));
    assert (x0.setCell(CPos("B4"), "=($A1+A$2)^2"));
    assert (x0.setCell(CPos("B5"), "=B1+B2+B3+B4"));
    assert (x0.setCell(CPos("B6"), "=B1+B2+B3+B4+B5"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(625.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-110.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(1024.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(930.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(2469.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(4938.0)));
    assert (x0.setCell(CPos("A1"), "12"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(627.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-154.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(1056.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(5625.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(11250.0)));
    x1 = x0;
    assert (x0.setCell(CPos("A2"), "100"));
    assert (x1.setCell(CPos("A2"), "=A3+A5+A4"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(38916.0)));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3612.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-204.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(17424.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(24928.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(49856.0)));
    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
//    std::cout<<"\n--------------------------------\n";
//    std::cout<<data;
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    assert (x0.setCell(CPos("A3"), "4e1"));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    for (size_t i = 0; i < std::min<size_t>(data.length(), 10); i++)
        data[i] ^= 0x5a;
    iss.clear();
    iss.str(data);
    assert (!x1.load(iss));
    assert (x0.setCell(CPos("D0"), "10"));
    assert (x0.setCell(CPos("D1"), "20"));
    assert (x0.setCell(CPos("D2"), "30"));
    assert (x0.setCell(CPos("D3"), "40"));
    assert (x0.setCell(CPos("D4"), "50"));
    assert (x0.setCell(CPos("E0"), "60"));
    assert (x0.setCell(CPos("E1"), "70"));
    assert (x0.setCell(CPos("E2"), "80"));
    assert (x0.setCell(CPos("E3"), "90"));
    assert (x0.setCell(CPos("E4"), "100"));
    assert (x0.setCell(CPos("F10"), "=D0+5"));
    assert (x0.setCell(CPos("F11"), "=$D0+5"));
    assert (x0.setCell(CPos("F12"), "=D$0+5"));
    assert (x0.setCell(CPos("F13"), "=$D$0+5"));
    x0.copyRect(CPos("G11"), CPos("F10"), 1, 4);
    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    x0.copyRect(CPos("G11"), CPos("F10"), 2, 4);
    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("H10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H11")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H12")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(35.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue()));
    assert (x0.setCell(CPos("F0"), "-27"));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    x0.copyRect(CPos("H12"), CPos("H13"), 1, 2);
    assert (valueMatch(x0.getValue(CPos("H12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(-22.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
