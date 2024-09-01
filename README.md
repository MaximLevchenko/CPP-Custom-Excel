# **Spreadsheet Processor Project** 

## **Overview**

This project is a comprehensive C++ implementation of a spreadsheet processor, similar to widely-used software like Microsoft Excel or Google Sheets. It supports a range of functionalities including arithmetic operations, cell referencing (both absolute and relative), text handling, and complex expression evaluation. The project is designed to be modular and maintainable, allowing easy extension and modification.

## **Key Features**

- **Arithmetic Operations**: Perform addition, subtraction, multiplication, division, and exponentiation directly within cells.
- **Cell Referencing**: Supports both absolute (`$A$1`) and relative (`A1`) references, allowing for dynamic updates and formula recalculations.
- **Text Handling**: Cells can store and manage text, including special characters and quotes.
- **Expression Evaluation**: Evaluate complex expressions that reference other cells, enabling dynamic and powerful data manipulation.
- **Cyclic Dependency Detection**: Automatically detects and handles cyclic dependencies in cell references to prevent infinite loops.
- **Persistence**: Save and load the entire spreadsheet state, ensuring that data is preserved between sessions.

## **Core Classes**

### **`CSpreadsheet`**
The central class that manages the spreadsheet's state, processes cell operations, and handles the evaluation of expressions.

### **`CPos`**
Represents the position of a cell in the spreadsheet using a row and column index, supporting operations necessary for cell referencing and manipulation.

### **`ExprElement`**
An abstract base class representing elements that can be part of an expression, such as constants, operations, or references.

### **Derived Classes of `ExprElement`**:
- **`Constant`**: Represents a numeric constant.
- **`StringVariable`**: Handles string values in cells.
- **`Reference`**: Manages references to other cells.
- **`BinaryOperation`**: Represents binary operations like addition or multiplication.
- **`UnaryOperation`**: Represents unary operations like negation.
- **`Range`**: Handles ranges of cells.
- **`FunctionCall`**: Supports function calls within expressions.


## **Usage**

- **Setting Cell Values**: Cells can be assigned numbers, strings, or expressions.
- **Evaluating Expressions**: Expressions are evaluated based on the spreadsheet's current state.
- **Loading and Saving**: The spreadsheet's state can be saved to a file and loaded back to continue where you left off.

## Examples of Usage

Below are some examples demonstrating how to use the spreadsheet processor to perform various operations.

### 1. Basic Cell Operations

Set cell values to numbers, text, or formulas and retrieve the evaluated results:

```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("A1"), "10");
sheet.setCell(CPos("A2"), "20.5");
sheet.setCell(CPos("A3"), "3e1");  // Scientific notation for 30
sheet.setCell(CPos("A4"), "=40");  // Formula assigning value 40
sheet.setCell(CPos("A5"), "=5e+1"); // Formula with scientific notation
sheet.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n");
sheet.setCell(CPos("A7"), "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\"");

// Retrieve cell values
assert(valueMatch(sheet.getValue(CPos("A1")), CValue(10.0)));
assert(valueMatch(sheet.getValue(CPos("A2")), CValue(20.5)));
assert(valueMatch(sheet.getValue(CPos("A3")), CValue(30.0)));
assert(valueMatch(sheet.getValue(CPos("A4")), CValue(40.0)));
assert(valueMatch(sheet.getValue(CPos("A5")), CValue(50.0)));
assert(valueMatch(sheet.getValue(CPos("A6")), CValue("raw text with any characters, including a quote \" or a newline\n")));
assert(valueMatch(sheet.getValue(CPos("A7")), CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
```
### 2. Arithmetic and Formula Calculations
Set formulas and perform calculations:
```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("A1"), "10");
sheet.setCell(CPos("A2"), "20");
sheet.setCell(CPos("B1"), "=A1+A2*2");  // B1 = 10 + 20*2 = 50
sheet.setCell(CPos("B2"), "= -A1 ^ 2 - A2 / 2");  // B2 = -(10^2) - 20/2 = -110
sheet.setCell(CPos("B3"), "= 2 ^ $A$1");  // B3 = 2^10 = 1024

// Retrieve and validate the results
assert(valueMatch(sheet.getValue(CPos("B1")), CValue(50.0)));
assert(valueMatch(sheet.getValue(CPos("B2")), CValue(-110.0)));
assert(valueMatch(sheet.getValue(CPos("B3")), CValue(1024.0)));
```
### 3. Handling Cyclic Dependencies
Detect and handle cyclic dependencies between cells:
```cpp
CSpreadsheet sheet;
assert(sheet.setCell(CPos("A1"), "=A2")); // Sets A1 to reference A2
assert(sheet.setCell(CPos("A2"), "=A1")); // Sets A2 to reference A1

// Attempting to get the value should detect a cyclic dependency
try {
    sheet.getValue(CPos("A1"));
} catch (const std::logic_error &e) {
    std::cout << "Detected cyclic dependency: " << e.what() << std::endl;
}
```
### 4. Copying Ranges of Cells
Copy a range of cells and paste it into another location:
```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("D0"), "10");
sheet.setCell(CPos("D1"), "20");
sheet.setCell(CPos("F0"), "=D0+5");  // F0 = 10 + 5 = 15
sheet.copyRect(CPos("G0"), CPos("F0"), 1, 1);  // Copy F0 to G0

// Validate copied values
assert(valueMatch(sheet.getValue(CPos("G0")), CValue(15.0)));
```
### 5. Saving and Loading Spreadsheets
Persist the spreadsheet to a stream and restore it from the stream:
```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("A1"), "10");
std::ostringstream savedState;
sheet.save(savedState);  // Save the state

std::istringstream toLoad(savedState.str());
sheet.load(toLoad);  // Load the saved state

assert(valueMatch(sheet.getValue(CPos("A1")), CValue(10.0)));  // Validate restored value
```
## **Building and Running**

The project is organized into separate source files for clarity and maintainability. You can build and run the project using the provided Makefile.

### **Using Makefile**

1. **Build the project**:
   ```bash
   make
   ```
2. **Run the executable**:
```bash
./excel
```
The Makefile handles all necessary dependencies and compilation flags.
3. **Clean the build:**
```bash
make clean
```

## Conclusion
This spreadsheet processor project demonstrates the power of C++ in building complex, maintainable, and efficient software applications.
