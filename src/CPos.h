#ifndef CPOS_H
#define CPOS_H

#include "main.h"

/**
 * @class CPos
 * @brief Represents a position in a spreadsheet using column and row identifiers.
 *
 * The CPos class provides methods for handling spreadsheet cell references,
 * including converting from string references, calculating offsets, and more.
 */
class CPos {
public:
    /**
     * @brief Constructs a CPos object from a string cell reference.
     *
     * This constructor interprets a string in the format of Excel-like cell
     * references (e.g., "A1", "B2") and converts it into column and row indices.
     *
     * @param str A string representing the cell reference.
     */
    CPos(std::string_view str);

    /**
     * @brief Constructs a CPos object from column and row numbers.
     *
     * This constructor directly sets the column and row indices for the position.
     *
     * @param col The column number (starting from 1 for "A").
     * @param row The row number (starting from 1).
     */
    CPos(size_t col, size_t row);

    /**
     * @brief Calculates the offset between two positions.
     *
     * This operator subtracts another CPos from the current one, resulting in
     * a new CPos that represents the offset in columns and rows.
     *
     * @param other The other CPos object to subtract.
     * @return CPos The resulting offset position.
     */
    CPos operator-(const CPos &other) const;

    /**
     * @brief Returns a new CPos shifted by the specified offsets.
     *
     * This method shifts the current position by the specified amounts in both
     * the x (columns) and y (rows) directions.
     *
     * @param dx The shift in the x direction (columns).
     * @param dy The shift in the y direction (rows).
     * @return CPos The new position after shifting.
     */
    CPos shift(int dx, int dy) const;

    /**
     * @brief Gets the unique identifier for the position.
     *
     * This method calculates a unique identifier for the position, which is a
     * combination of the column and row numbers. Useful for storing and accessing
     * cell positions in data structures like hash maps.
     *
     * @return size_t The unique identifier for the position.
     */
    size_t getUniqueId() const;

    /**
     * @brief Gets the column number.
     *
     * @return size_t The column number.
     */
    size_t getColumn() const;

    /**
     * @brief Gets the row number.
     *
     * @return size_t The row number.
     */
    size_t getRow() const;

private:
    size_t column, row;  ///< The column and row indices for the position.
    size_t uniqueId;     ///< A unique identifier for the position.

    /**
     * @brief Updates the unique identifier for the position.
     *
     * This method calculates and updates the uniqueId member variable based on
     * the current column and row indices.
     */
    void updateUniqueId();

    /**
     * @brief Parses a string cell reference and sets the column and row.
     *
     * This method interprets a string in the format of Excel-like cell
     * references and sets the column and row indices accordingly.
     *
     * @param cellRef A string representing the cell reference.
     */
    void parseCellRef(std::string_view cellRef);
};

#endif // CPOS_H