# Spreadsheet Processor Project

## Overview

This C++ project provides a robust implementation of a spreadsheet processor, simulating functionality typical of software like Microsoft Excel or Google Sheets. The processor supports basic arithmetic operations, cell referencing (absolute and relative), text handling, and expression evaluation.

## Features

- **Basic Arithmetic Operations**: Supports addition, subtraction, multiplication, division, and exponentiation.
- **Cell Referencing**: Both absolute (`$A$1`) and relative (`A1`) cell references are supported.
- **Text Handling**: Cells can contain text, including special characters and quotes.
- **Expression Evaluation**: Complex expressions can be evaluated based on the content of other cells.
- **Cyclic Dependency Detection**: The system detects cyclic dependencies and handles them gracefully.
- **Persistence**: Ability to save and load the state of the spreadsheet to ensure data is not lost between sessions.

## Core Classes

### `CSpreadsheet`
The primary class that acts as the spreadsheet processor, handling cell operations, and managing the state.

### `CPos`
Represents cell positions using a row and column index, supporting operations necessary for cell referencing and manipulation.

### `ExprElement`
An abstract base class for elements that can be part of an expression (e.g., constants, operations).

### `Constant`, `StringVariable`, `Reference`, `BinaryOperation`, `UnaryOperation`, `Range`, `FunctionCall`
Concrete classes derived from `ExprElement` that represent different types of operations and values within a cell.

## Building and Running

The project has been refactored into separate source files for better organization and maintainability. To compile the project, use the provided Makefile, which manages dependencies and compilation flags.

### Using Makefile

1. **Build the project:**
   ```bash
   make
   ```

The Makefile automatically includes all necessary source files and links with the required libraries.

2. **Clean the build:**
   ```bash
   make clean
   ```  
3. **Direct Compilation:**

Alternatively, you can compile manually using a command like:
   ```bash
  g++ -std=c++20 -Wall -pedantic -g -o excel -fsanitize=address main.cpp CSpreadsheet.cpp CPos.cpp ExprElement.cpp CustomExpressionBuilder.cpp -L./x86_64-linux-gnu -lexpression_parser
   ```

## Usage

- **Setting Cell Values**: Cells can be set to numbers, strings, or expressions.
- **Evaluating Expressions**: Expressions are evaluated based on the context of the spreadsheet.
- **Loading and Saving**: The spreadsheet's state can be saved to a file and loaded back to resume work.

#### Example Code

```cpp
CSpreadsheet sheet;
sheet.setCell(CPos("A1"), "10");
sheet.setCell(CPos("A2"), "=A1*2");
CValue value = sheet.getValue(CPos("A2")); // Expected to be 20
```

## Persistence

- **Saving**: : The spreadsheet's complete state, including expressions and cell values, can be serialized to a stream.
- **Loading**: : Deserialize the stream back to restore the state.

#### Example Code

```cpp
std::ostringstream savedState;
sheet.save(savedState);
std::istringstream toLoad(savedState.str());
sheet.load(toLoad);
```




