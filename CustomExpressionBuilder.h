#ifndef CUSTOM_EXPRESSION_BUILDER_H
#define CUSTOM_EXPRESSION_BUILDER_H

#include "main.h"
#include "ExprElement.h"

/**
 * @class CustomExpressionBuilder
 * @brief A custom builder class for constructing expressions in the spreadsheet.
 *
 * The CustomExpressionBuilder class extends the CExprBuilder interface to provide
 * implementations for building various types of expressions, including arithmetic
 * operations, comparisons, and functions. It uses a stack-based approach to construct
 * the expression elements.
 */
class CustomExpressionBuilder : public CExprBuilder {
public:
    /**
     * @brief Adds an addition operation to the expression.
     *
     * Pushes a BinaryOperation representing addition onto the expression stack.
     */
    void opAdd() override;

    /**
     * @brief Adds a subtraction operation to the expression.
     *
     * Pushes a BinaryOperation representing subtraction onto the expression stack.
     */
    void opSub() override;

    /**
     * @brief Adds a multiplication operation to the expression.
     *
     * Pushes a BinaryOperation representing multiplication onto the expression stack.
     */
    void opMul() override;

    /**
     * @brief Adds a division operation to the expression.
     *
     * Pushes a BinaryOperation representing division onto the expression stack.
     */
    void opDiv() override;

    /**
     * @brief Adds an exponentiation operation to the expression.
     *
     * Pushes a BinaryOperation representing exponentiation onto the expression stack.
     */
    void opPow() override;

    /**
     * @brief Adds a negation operation to the expression.
     *
     * Pushes a UnaryOperation representing negation onto the expression stack.
     */
    void opNeg() override;

    /**
     * @brief Adds an equality comparison to the expression.
     *
     * Pushes a BinaryOperation representing equality onto the expression stack.
     */
    void opEq() override;

    /**
     * @brief Adds a not-equal comparison to the expression.
     *
     * Pushes a BinaryOperation representing not-equal onto the expression stack.
     */
    void opNe() override;

    /**
     * @brief Adds a less-than comparison to the expression.
     *
     * Pushes a BinaryOperation representing less-than onto the expression stack.
     */
    void opLt() override;

    /**
     * @brief Adds a less-than-or-equal comparison to the expression.
     *
     * Pushes a BinaryOperation representing less-than-or-equal onto the expression stack.
     */
    void opLe() override;

    /**
     * @brief Adds a greater-than comparison to the expression.
     *
     * Pushes a BinaryOperation representing greater-than onto the expression stack.
     */
    void opGt() override;

    /**
     * @brief Adds a greater-than-or-equal comparison to the expression.
     *
     * Pushes a BinaryOperation representing greater-than-or-equal onto the expression stack.
     */
    void opGe() override;

    /**
     * @brief Adds a numeric value to the expression.
     *
     * Pushes a Constant representing a numeric value onto the expression stack.
     *
     * @param val The numeric value to add to the expression.
     */
    void valNumber(double val) override;

    /**
     * @brief Adds a string value to the expression.
     *
     * Pushes a StringVariable representing a string onto the expression stack.
     *
     * @param val The string value to add to the expression.
     */
    void valString(std::string val) override;

    /**
     * @brief Adds a cell reference to the expression.
     *
     * Pushes a Reference representing a cell reference onto the expression stack.
     *
     * @param val The cell reference to add to the expression.
     */
    void valReference(std::string val) override;

    /**
     * @brief Adds a range reference to the expression.
     *
     * Pushes a Range representing a range reference onto the expression stack.
     *
     * @param val The range reference to add to the expression.
     */
    void valRange(std::string val) override;

    /**
     * @brief Adds a function call to the expression.
     *
     * Pushes a FunctionCall representing a function call onto the expression stack.
     *
     * @param fnName The name of the function.
     * @param paramCount The number of parameters the function takes.
     */
    void funcCall(std::string fnName, int paramCount) override;

    /**
     * @brief Retrieves the constructed expression.
     *
     * Returns a constant reference to the stack containing the constructed expression
     * elements, which can be evaluated or further manipulated.
     *
     * @return const std::stack<std::shared_ptr<ExprElement>>& The expression stack.
     */
    const std::stack<std::shared_ptr<ExprElement>> &getExpression() const;

private:
    std::stack<std::shared_ptr<ExprElement>> expression; ///< Stack holding the constructed expression elements.
};

#endif // CUSTOM_EXPRESSION_BUILDER_H
