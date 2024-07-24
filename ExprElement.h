#ifndef EXPR_ELEMENT_H
#define EXPR_ELEMENT_H

#include "main.h"
#include "CPos.h"

class ExprElement;

// Custom type definition for cell values, supporting various types including expressions
using CustomCValue = std::variant<std::monostate, double, std::string, int, std::stack<std::shared_ptr<ExprElement>>>;

/**
 * @class ExprElement
 * @brief An abstract base class representing elements in an expression.
 *
 * The ExprElement class serves as the base for various expression elements, such as constants,
 * variables, operations, ranges, and references. Each derived class must implement methods
 * for evaluating the expression and saving its representation.
 */
class ExprElement {
public:
    virtual ~ExprElement() = default;

    /**
     * @brief Evaluates the expression element and pushes the result onto the stack.
     *
     * This method must be implemented by derived classes to evaluate the expression
     * element and push the result onto the evaluation stack.
     *
     * @param evalStack The stack used to hold evaluation results.
     * @param sheet The map representing the spreadsheet, where cells are identified by unique IDs.
     * @param evaluationPath A set used to track the path of evaluations to detect cyclic dependencies.
     */
    virtual void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                          std::unordered_set<size_t> &evaluationPath) const = 0;

    /**
     * @brief Saves the expression element as a string.
     *
     * This method must be implemented by derived classes to provide a string representation
     * of the expression element, suitable for saving or displaying.
     *
     * @return std::string A string representation of the expression element.
     */
    virtual std::string save() const = 0;
};

/**
 * @class Constant
 * @brief Represents a constant numerical value in an expression.
 */
class Constant : public ExprElement {
    double value; ///< The numerical value of the constant.
public:
    explicit Constant(double val);

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    std::string save() const override;
};

/**
 * @class StringVariable
 * @brief Represents a string variable in an expression.
 */
class StringVariable : public ExprElement {
    std::string name; ///< The string value of the variable.
public:
    explicit StringVariable(std::string name);

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    std::string save() const override;
};

/**
 * @class BinaryOperation
 * @brief Represents a binary operation (e.g., addition, subtraction) in an expression.
 */
class BinaryOperation : public ExprElement {
    std::string op; ///< The operator symbol (e.g., "+", "-", "*").
public:
    explicit BinaryOperation(std::string op);

    std::string getOp() const;
    std::string save() const override;

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    CValue perform(const std::string &op, const CValue &left, const CValue &right) const;
};

/**
 * @class UnaryOperation
 * @brief Represents a unary operation (e.g., negation) in an expression.
 */
class UnaryOperation : public ExprElement {
    std::string op; ///< The operator symbol (e.g., "-").
public:
    explicit UnaryOperation(std::string op);

    std::string getOp() const;
    std::string save() const override;

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    double apply(const std::string &op, double value) const;
};

/**
 * @class Range
 * @brief Represents a range of cells in a spreadsheet.
 */
class Range : public ExprElement {
    std::string rangeRef; ///< The string representation of the cell range (e.g., "A1:B2").
public:
    explicit Range(std::string rangeRef);

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    std::string save() const override;
};

/**
 * @class FunctionCall
 * @brief Represents a function call in an expression.
 */
class FunctionCall : public ExprElement {
    std::string functionName; ///< The name of the function.
    size_t parameterCount;    ///< The number of parameters the function takes.
public:
    FunctionCall(std::string fnName, size_t paramCount);

    std::string save() const override;

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;
};

/**
 * @class Reference
 * @brief Represents a reference to another cell in a spreadsheet.
 */
class Reference : public ExprElement {
    size_t row, column;       ///< The row and column indices of the referenced cell.
    bool isAbsoluteRow, isAbsoluteColumn; ///< Flags indicating if the row/column are absolute.
    std::string cellReference; ///< The string representation of the cell reference.

public:
    explicit Reference(std::string cellRef);

    std::string save() const override;

    void evaluate(std::stack<CValue> &evalStack, const std::unordered_map<size_t, CustomCValue> &sheet,
                  std::unordered_set<size_t> &evaluationPath) const override;

    /**
     * @brief Adjusts the reference for relative movement.
     *
     * This method updates the row and column indices for relative references based on the given offset.
     *
     * @param offset The CPos object representing the offset to apply.
     */
    void moveRelativeReferencesBy(const CPos &offset);

private:
    /**
     * @brief Parses a string cell reference to determine row and column indices.
     *
     * This method interprets a string in the format of Excel-like cell references and sets
     * the row and column indices accordingly.
     *
     * @param ref The string representation of the cell reference.
     */
    void parseReference(const std::string &ref);

    /**
     * @brief Updates the string representation of the cell reference.
     *
     * This method rebuilds the cellReference string based on the current row and column indices,
     * taking into account whether they are absolute or relative.
     */
    void updateCellReferenceString();
};

#endif // EXPR_ELEMENT_H
