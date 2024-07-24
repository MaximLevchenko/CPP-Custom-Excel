#ifndef CSPREADSHEET_H
#define CSPREADSHEET_H

#include "main.h"
#include "CPos.h"
#include "ExprElement.h"

// Custom type definition for spreadsheet cell values
using CustomCValue = std::variant<std::monostate, double, std::string, int, std::stack<std::shared_ptr<ExprElement>>>;

/**
 * @class CSpreadsheet
 * @brief Represents a spreadsheet capable of storing and evaluating cell contents.
 *
 * The CSpreadsheet class provides functionalities to manipulate and evaluate
 * cell contents, which can include numbers, strings, and expressions. It supports
 * operations such as loading from and saving to a stream, setting and retrieving cell values,
 * and copying rectangular regions of cells.
 */
class CSpreadsheet {
public:
    /**
     * @brief Returns the capabilities of the spreadsheet implementation.
     *
     * This function provides information about the supported features, such as handling
     * cyclic dependencies, file input/output, and optimized performance.
     *
     * @return unsigned A bitmask indicating the supported features.
     */
    static unsigned capabilities();

    /**
     * @brief Constructs an empty CSpreadsheet object.
     */
    CSpreadsheet();

    /**
     * @brief Loads spreadsheet data from an input stream.
     *
     * This function reads data from the provided input stream, initializes the
     * spreadsheet with the data, and verifies the integrity using a checksum.
     *
     * @param is An input stream containing the spreadsheet data.
     * @return bool True if the data is successfully loaded and verified, false otherwise.
     */
    bool load(std::istream &is);

    /**
     * @brief Saves the current spreadsheet data to an output stream.
     *
     * This function writes the current state of the spreadsheet to the provided
     * output stream, including a checksum for data integrity verification.
     *
     * @param os An output stream to write the spreadsheet data to.
     * @return bool True if the data is successfully saved, false otherwise.
     */
    bool save(std::ostream &os) const;

    /**
     * @brief Sets the contents of a specific cell.
     *
     * This function sets the contents of a cell at the given position. The contents
     * can be a direct value or an expression. If the contents start with '=', they
     * are treated as an expression and parsed accordingly.
     *
     * @param pos The position of the cell to set.
     * @param contents A string representing the cell's contents.
     * @return bool True if the cell contents are successfully set, false otherwise.
     */
    bool setCell(const CPos &pos, const std::string &contents);

    /**
     * @brief Retrieves the evaluated value of a specific cell.
     *
     * This function returns the evaluated value of the cell at the given position.
     * If the cell contains an expression, it is evaluated, and the result is returned.
     *
     * @param pos The position of the cell to retrieve the value from.
     * @return CValue The evaluated value of the cell.
     */
    CValue getValue(const CPos &pos) const;

    /**
     * @brief Copies a rectangular region of cells from one area to another.
     *
     * This function copies a rectangular region of cells, defined by a source position
     * and dimensions, to a destination position. It correctly handles references and
     * updates them to maintain consistency.
     *
     * @param dst The destination position to copy the region to.
     * @param src The source position of the region to copy.
     * @param w The width of the region (number of columns).
     * @param h The height of the region (number of rows).
     */
    void copyRect(CPos dst, CPos src, int w = 1, int h = 1);

private:
    /**
     * @brief Determines the value type of the provided contents string.
     *
     * This function analyzes the contents string and determines whether it
     * represents a number, a string, or an expression. It returns the appropriate
     * CustomCValue variant based on the analysis.
     *
     * @param contents A string containing the value or expression.
     * @return CustomCValue The determined value type.
     */
    CustomCValue DetermineValue(const std::string &contents);

    std::unordered_map<size_t, CustomCValue> sheet; ///< The internal storage for cell values and expressions.
    mutable std::unordered_set<size_t> evaluationPath; ///< A set used to track evaluation paths for detecting cyclic dependencies.
};

#endif // CSPREADSHEET_H