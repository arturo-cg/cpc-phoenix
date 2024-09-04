#pragma once

/**
** It holds a single line of code.
** This is the base struct, there is one derived struct for type of line.
*/
struct ProgramCodeLine
{
    enum class Type
    {
        Comment,
        Directive,
        Instruction,
    };

    ProgramCodeLine(Type type, int lineNumber) : _type(type), _lineNumber(lineNumber) { }
    virtual ~ProgramCodeLine() { }

    Type _type;
    int _lineNumber;
};

/**
** It holds a line of comment.
*/
struct ProgramCodeCommentLine : ProgramCodeLine
{
    ProgramCodeCommentLine(int lineNumber, const std::string& comment) : ProgramCodeLine(Type::Comment, lineNumber), _comment(comment) { }

    std::string _comment;
};

/**
** It holds a line of an assembler directive.
*/
struct ProgramCodeDirectiveLine : ProgramCodeLine
{
    ProgramCodeDirectiveLine(int lineNumber, const std::string& directive, const std::string& operands) : ProgramCodeLine(Type::Directive, lineNumber), _directive(directive), _operands(operands) { }

    std::string _directive;
    std::string _operands;
};

/**
** It holds a line of an instruction.
*/
struct ProgramCodeInstructionLine : ProgramCodeLine
{
    ProgramCodeInstructionLine(int lineNumber, cpcWord address, const std::string& operation, const std::string& operands) : ProgramCodeLine(Type::Instruction, lineNumber), _address(address), _operation(operation), _operands(operands) { }

    cpcWord _address;
    std::string _operation;
    std::string _operands;
};

/**
** It holds the program code collected so far by the Program Analyzer.
*/
class ProgramCode
{
public:

    ProgramCode() { m_bOk = false; }
    virtual ~ProgramCode() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    // Clears the stored program code, leaving it empty.
    void Clear();

    // Adds a blank line on a new line at the end of the program code.
    void AddBlankLine();
    // Adds one or more blank lines on a new line at the end of the program code.
    void AddBlankLines(int numBlankLines);
    // Adds a comment on a new line at the end of the program code.
    void AddComment(const std::string& comment);
    // Adds a directive on a new line at the end of the program code.
    void AddDirective(const std::string& directive, const std::string& operands);
    // Adds an instruction on a new line at the end of the program code.
    void AddInstruction(cpcWord address, const std::string& operation, const std::string& operands/*, const std::string& comment*/);

    void DrawGui();

private:

    struct CodeStyle
    {
        int addressWidth;                           // Width, in characters, of the address column.
        int instructionOperationWidth;              // Width, in characters, of the instruction's operation column.
    };

    using CodeLineList = vector<ProgramCodeLine*>;

    static const int FirstLineNumber = 1;

    void ResetVars();
    void FreeVars();

    void SetDefaultCodeStyle();

    //static void AppendStringFormat(std::string* str, const char* format, ...);

    bool m_bOk;
    CodeStyle m_codeStyle;
    CodeLineList m_codeLines;       // Only non-blank lines are stored.
    int m_nextLineNumber;
    bool m_isDirty;
};
