#include "CPos.h"

/**
 * Constructs a CPos object from a string cell reference.
 *
 * @param str A string representing the cell reference.
 */
CPos::CPos(std::string_view str) {
    parseCellRef(str);
    updateUniqueId();  // Compute unique ID based on column and row.
}

/**
 * Constructs a CPos object from column and row numbers.
 *
 * @param col The column number.
 * @param row The row number.
 */
CPos::CPos(size_t col, size_t row) : column(col), row(row) {
    updateUniqueId();  // Compute unique ID based on column and row.
}

/**
 * Calculates the offset between two positions.
 *
 * @param other The other CPos object to subtract.
 * @return CPos The resulting offset position.
 */
CPos CPos::operator-(const CPos &other) const {
    return CPos(column - other.column, row - other.row);
}

/**
 * Returns a new CPos shifted by the specified offsets.
 *
 * @param dx The shift in the x direction (columns).
 * @param dy The shift in the y direction (rows).
 * @return CPos The new position after shifting.
 */
CPos CPos::shift(int dx, int dy) const {
    return CPos(column + dx, row + dy);
}

/**
 * Gets the unique identifier for the position.
 *
 * @return size_t The unique identifier.
 */
size_t CPos::getUniqueId() const {
    return uniqueId;
}

/**
 * Gets the column number.
 *
 * @return size_t The column number.
 */
size_t CPos::getColumn() const { return column; }

/**
 * Gets the row number.
 *
 * @return size_t The row number.
 */
size_t CPos::getRow() const { return row; }

/**
 * Updates the unique identifier for the position.
 *
 * Combines column and row into a unique identifier.
 */
void CPos::updateUniqueId() {
    uniqueId = (column << (sizeof(size_t) * 4)) + row;
}

/**
 * Parses a string cell reference and sets the column and row.
 *
 * Converts Excel-like cell references into numerical indices.
 *
 * @param cellRef A string representing the cell reference.
 */
void CPos::parseCellRef(std::string_view cellRef) {
    size_t i = 0;
    column = 0;

    // Process column characters (base-26 conversion).
    while (i < cellRef.size() && std::isalpha(cellRef[i])) {
        column = column * 26 + (toupper(cellRef[i]) - 'A' + 1);
        ++i;
    }

    // Ensure row numbers are after the column letters.
    if (i < cellRef.size()) {
        row = std::stoull(std::string(cellRef.substr(i)));
    } else {
        throw std::invalid_argument("Invalid cell reference format");
    }
}