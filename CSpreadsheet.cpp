#include "CSpreadsheet.h"
#include "ExprElement.h"
#include "CustomExpressionBuilder.h"

// Evaluates an expression stack and returns the resulting value.
static CValue evaluateExpression(const std::stack<std::shared_ptr<ExprElement>> &exprStack,
                                 const std::unordered_map<size_t, CustomCValue> &sheet,
                                 std::unordered_set<size_t> &evaluationPath) {
    std::stack<std::shared_ptr<ExprElement>> copyExprStack;
    auto tempStack = exprStack;

    // Reverse the stack for correct evaluation order.
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
            // Return default value on error.
            return CValue();
        }
    }

    if (evalStack.size() != 1) {
        throw std::runtime_error("Invalid expression: more than one value remains after evaluation");
    }

    return evalStack.top();
}

unsigned CSpreadsheet::capabilities() {
    return SPREADSHEET_CYCLIC_DEPS | SPREADSHEET_FILE_IO | SPREADSHEET_SPEED;
}

CSpreadsheet::CSpreadsheet() = default;

bool CSpreadsheet::load(std::istream &is) {
    std::string line;
    unsigned long calculatedChecksum = 0;
    unsigned long readChecksum;

    if (!getline(is, line)) return false;
    std::istringstream checksumStream(line);
    std::string checksumLabel;
    checksumStream >> checksumLabel >> readChecksum;
    if (checksumLabel != "CHECKSUM") return false;

    std::ostringstream contentStream;

    // Process content and calculate checksum.
    while (getline(is, line)) {
        contentStream << line << '\n';
        for (char c : line) {
            calculatedChecksum += static_cast<unsigned char>(c);
        }
        calculatedChecksum += '\n';
    }

    if (readChecksum != calculatedChecksum) {
        return false;
    }

    std::istringstream dataStream(contentStream.str());
    while (getline(dataStream, line)) {
        std::stringstream ss(line);
        size_t key;
        char delim;
        ss >> key >> delim;
        ss.get();

        if (ss.peek() == '[') {
            // Process expression stack.
            ss.get();
            std::stack<std::shared_ptr<ExprElement>> exprStack;
            std::string element;

            while (getline(ss, element, ',')) {
                element.erase(0, element.find_first_not_of(" "));
                if (element.back() == ']') {
                    element.pop_back();
                }

                std::stringstream elemStream(element);
                std::string type;
                elemStream >> type >> std::ws;

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
                    std::getline(elemStream, content, '"');
                    std::getline(elemStream, content, '"');
                    size_t pos = 0;
                    while ((pos = content.find("\"\"", pos)) != std::string::npos) {
                        content.replace(pos, 2, "\"");
                        pos += 1;
                    }
                    exprStack.push(std::make_shared<StringVariable>(content));
                } else if (type == "Range") {
                    std::string range;
                    elemStream >> range;
                    exprStack.push(std::make_shared<Range>(range));
                } else if (type == "Function") {
                    std::string functionName;
                    double paramCount;
                    elemStream >> functionName >> paramCount;
                    exprStack.push(std::make_shared<FunctionCall>(functionName, paramCount));
                }
            }
            sheet[key] = exprStack;
        } else {
            // Process single values.
            if (ss.peek() == '"') {
                std::string strValue;
                getline(ss, strValue, '"');
                getline(ss, strValue, '"');
                size_t pos = 0;
                while ((pos = strValue.find("\"\"", pos)) != std::string::npos) {
                    strValue.replace(pos, 2, "\"");
                    pos += 1;
                }
                sheet[key] = strValue;
            } else if (ss.peek() != 'u') {
                double numValue;
                ss >> numValue;
                sheet[key] = numValue;
            } else {
                sheet[key] = std::monostate();
            }
        }
    }
    return true;
}

bool CSpreadsheet::save(std::ostream &os) const {
    std::ostringstream contentStream;
    unsigned long checksum = 0;

    for (const auto &[key, val] : sheet) {
        contentStream << key << ", ";
        if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(val)) {
            std::stack<std::shared_ptr<ExprElement>> tempStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(val);
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
            for (char ch : std::get<std::string>(val)) {
                if (ch == '"') {
                    escapedString += "\"\"";
                } else {
                    escapedString += ch;
                }
            }
            contentStream << '"' << escapedString << '"';
        } else {
            contentStream << "undefined";
        }
        contentStream << std::endl;
    }

    std::string data = contentStream.str();
    for (char c : data) {
        checksum += static_cast<unsigned char>(c);
    }

    os << "CHECKSUM " << checksum << std::endl;
    os << data;
    return true;
}

bool CSpreadsheet::setCell(const CPos &pos, const std::string &contents) {
    if (!contents.empty() && contents[0] == '=') {
        CustomExpressionBuilder exprBuilder;
        parseExpression(contents, exprBuilder);
        CustomCValue expression = exprBuilder.getExpression();
        sheet[pos.getUniqueId()] = expression;
    } else {
        CustomCValue value = DetermineValue(contents);
        sheet[pos.getUniqueId()] = value;
    }
    return true;
}

CValue CSpreadsheet::getValue(const CPos &pos) const {
    auto it = sheet.find(pos.getUniqueId());
    size_t uniqueId = pos.getUniqueId();

    if (it != sheet.end()) {
        if (!std::holds_alternative<std::monostate>(it->second)) {
            if (std::holds_alternative<double>(it->second)) {
                return std::get<double>(it->second);
            } else if (std::holds_alternative<std::string>(it->second)) {
                return std::get<std::string>(it->second);
            } else if (std::holds_alternative<int>(it->second)) {
                return static_cast<double>(std::get<int>(it->second));
            } else if (std::holds_alternative<std::stack<std::shared_ptr<ExprElement>>>(it->second)) {
                if (!evaluationPath.insert(uniqueId).second) {
                    evaluationPath.clear();
                    throw std::runtime_error("Cyclic dependency detected!");
                }
                try {
                    const auto &exprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(it->second);
                    CValue result = evaluateExpression(exprStack, sheet, evaluationPath);
                    evaluationPath.clear();
                    return result;
                } catch (const std::exception &e) {
                    evaluationPath.clear();
                    return CValue();
                }
            }
        }
    }
    evaluationPath.clear();
    return CValue();
}

void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h) {
    std::vector<std::pair<size_t, CustomCValue>> tempStorage;
    tempStorage.reserve(w * h);

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
                    std::stack<std::shared_ptr<ExprElement>> exprCopy;
                    std::stack<std::shared_ptr<ExprElement>> originalExprStack = std::get<std::stack<std::shared_ptr<ExprElement>>>(content);

                    std::stack<std::shared_ptr<ExprElement>> reversedCopy;
                    while (!originalExprStack.empty()) {
                        reversedCopy.push(originalExprStack.top());
                        originalExprStack.pop();
                    }

                    while (!reversedCopy.empty()) {
                        auto &elem = reversedCopy.top();
                        if (auto ref = dynamic_cast<Reference *>(elem.get())) {
                            auto copiedRef = std::make_shared<Reference>(*ref);
                            copiedRef->moveRelativeReferencesBy(offset);
                            exprCopy.push(copiedRef);
                        } else {
                            exprCopy.push(elem);
                        }
                        reversedCopy.pop();
                    }

                    tempStorage.emplace_back(toId, std::move(exprCopy));
                } else {
                    tempStorage.emplace_back(toId, content);
                }
            } else {
                tempStorage.emplace_back(toId, CustomCValue());
            }
        }
    }

    for (auto &[id, val] : tempStorage) {
        sheet[id] = std::move(val);
    }
}

CustomCValue CSpreadsheet::DetermineValue(const std::string &contents) {
    if (contents.empty()) {
        return std::monostate();
    }
    try {
        size_t idx;
        double numVal = std::stod(contents, &idx);
        if (idx == contents.size()) {
            return numVal;
        }
    } catch (const std::invalid_argument &) {
    } catch (const std::out_of_range &) {
    }
    return contents;
}