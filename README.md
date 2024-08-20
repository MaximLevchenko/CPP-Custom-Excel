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

## **Usage**

- **Setting Cell Values**: Cells can be assigned numbers, strings, or expressions.
- **Evaluating Expressions**: Expressions are evaluated based on the spreadsheet's current state.
- **Loading and Saving**: The spreadsheet's state can be saved to a file and loaded back to continue where you left off.

### **Example Code**

```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("A1"), "10");
sheet.setCell(CPos("A2"), "=A1*2");
CValue value = sheet.getValue(CPos("A2")); // Expected to be 20
```

## **Persistence**

- **Saving**: Serialize the spreadsheet's entire state, including cell values and expressions, to a stream.
- **Loading**: Deserialize a saved state from a stream to restore the spreadsheet.

### **Example Code**

```cpp
std::ostringstream savedState;
sheet.save(savedState);
std::istringstream toLoad(savedState.str());
sheet.load(toLoad);
```
## Conclusion
This spreadsheet processor project demonstrates the power of C++ in building complex, maintainable, and efficient software applications.
