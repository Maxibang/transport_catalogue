#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    
    if (c != ']') {
        throw ParsingError("LoadArray Error"s);
    }
    return Node(move(result));
}  
  
    
std::string LoadStringS(std::istream& input); 
    
Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        string key = LoadStringS(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    
    if (c != '}') {
        throw ParsingError("LoadDict Error"s);
    }
    
    return Node(move(result));
}
    
   
using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}
    
    
// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadStringS(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
   // cout << "\n\n LOADED STRING: " << s << " size: " << s.size() <<"\n\n";   
    return s;
}
    
    
// Get an alphabetic character words from input
std::string ParseWord(istream& input) {
    char ch;
    string str;
    while (input >> ch && std::isalpha(ch)) {
        str += ch;
    }
    input.putback(ch);
    return  str;
}  
    

// Loads bool-string ("true" || "false") from input
Node LoadBool(istream& input) {
    const auto word = ParseWord(input);
    
    if (word == "true"s) {
        return Node(true);
    }
    
    if (word == "false"s) {
        return Node(false);
    }
    throw ParsingError("LoadBool Error"s);
}
  
    
// Loads "null"-string from input
Node LoadNull(istream& input) {
    
    if (ParseWord(input) == "null"s) {
        return Node(nullptr);
    }
    
    throw ParsingError("LoadNull Error"s);
}
    
    
Node LoadNode(istream& input) {
    char c;
    input >> c;
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node(LoadStringS(input));
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 'f' || c == 't') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        auto tmp = LoadNumber(input);
        
        if (holds_alternative<int>(tmp)) {
            return Node(get<int>(tmp));
        }
        
        return Node(get<double>(tmp));
    }
}

}  // namespace
     
    
const Node::Value& Node::GetValue() const { 
    return value_; 
}
  
    
bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}
    
// Returns true, if in Node holds int or double.
bool Node::IsDouble() const {
    return holds_alternative<int>(value_) || holds_alternative<double>(value_);
}
    
    
// Returns true, if in Node holds double.   
bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}
    
    
bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}
    
    
bool Node::IsString() const {
    return holds_alternative<string>(value_);
}
    
    
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(value_);
}
    
    
bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}
   
    
bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}
    
    
Node::Node(Array array)
    : value_(move(array)) {
}

    
Node::Node(Dict map)
    : value_(move(map)) {
}


Node::Node(string value)
    : value_(move(value)) {
}


int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("It's not Int"s);
    } 
    return std::get<int>(value_);
}
    
    
bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("It's not Bool"s);
    } 
    return std::get<bool>(value_); 
}
   
    
// Возвращает значение типа double, если внутри хранится double либо int. 
// В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    if (IsInt()) {
        return AsInt();
    }
    if (IsDouble()) {
        return std::get<double>(value_); 
    }
    throw std::logic_error("It's not Double"s);
}
   
    
const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("It's not String"s);
    } 
    return std::get<std::string>(value_);
}
    
    
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("It's not Array"s);
    } 
    return std::get<Array>(value_);
}
    
    
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("It's not Map"s);
    } 
    return std::get<Dict>(value_);
}
    
    
Document::Document(Node root)
    : root_(move(root)) {
}

    
const Node& Document::GetRoot() const {
    return root_;
}

    
Document Load(istream& input) {
    return Document{LoadNode(input)};
}
    

// Шаблон, подходящий для вывода bool
void PrintValue(bool value, std::ostream& out) {
    //const auto bool_print = (value == true) ? "true"s : "false"s;
    out << std::boolalpha << value;
}

    
// Шаблон, подходящий для вывода bool
void PrintValue(const string &value, std::ostream& out) {
        out << "\""s;
        for (const auto& ch : value) { // Escape-symbols converter
            switch (ch) {
            case '"':
                out << "\\\""s;
                break;
            case '\\':
                out << "\\\\"s;
                break;
            case '\n':
                out << "\\n"s;
                break;
            case '\r':
                out << "\\r"s;
                break;
            case '\t':
                out << "\t"s;
                break;
            default:
                out << ch;
            }
        }
        out << "\""s; 
}
    
    
// Шаблон, подходящий для вывода double
void PrintValue(double value, std::ostream& out) {
    out << value;
}


// Шаблон, подходящий для вывода int
void PrintValue(int value, std::ostream& out) {
    out << value;
}

    
// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

    
 void PrintNode(const Node& node, std::ostream& out);
    
// Перегрузка функции PrintValue для вывода значений Array = std::vector<Node>
void PrintValue(const Array &arr, std::ostream& out) {
    out << '[';
    bool is_first = true;
    
    for (const auto& item : arr) {
        if (!is_first) {
            out << ", "s;
        }
        PrintNode(item, out);
        is_first = false;
    }
    
    out << ']';
}


// Перегрузка функции PrintValue для вывода значений Dict = std::map<std::string, Node>
void PrintValue(const Dict& dict, std::ostream& out) {
    out << "\n" << '{' << "   "s;
    
    bool is_first = true;
    for (const auto& [key, value] : dict) {
        if (!is_first) {
            out << ", "s;
        }
        out << '\"' << key << '\"' << ':';
        PrintNode(value, out);
        is_first = false;
        out << "\n";
    }
    out << '}' << "\n";
}
    

void PrintNode(const Node& node, std::ostream& out) {
    std::visit([&out](const auto& value){ PrintValue(value, out); }, node.GetValue());
} 

    
void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);    
}
    
    
json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

    
std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

}  // namespace json