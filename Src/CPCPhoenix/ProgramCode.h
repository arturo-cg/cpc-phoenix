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

    ProgramCodeLine(Type type) : _type(type) { }

    Type _type;
};

/**
** It holds a line of comment.
*/
struct ProgramCodeCommentLine : ProgramCodeLine
{
    ProgramCodeCommentLine(const std::string& comment) : ProgramCodeLine(Type::Comment), _comment(comment) { }

    std::string _comment;
};

/**
** It holds a line of an assembler directive.
*/
struct ProgramCodeDirectiveLine : ProgramCodeLine
{
    ProgramCodeDirectiveLine(const std::string& directive, const std::string& operands) : ProgramCodeLine(Type::Directive), _directive(directive), _operands(operands) { }

    std::string _directive;
    std::string _operands;
};

/**
** It holds a line of an instruction.
*/
struct ProgramCodeInstructionLine : ProgramCodeLine
{
    ProgramCodeInstructionLine(cpcWord address, const std::string& operation, const std::string& operands) : ProgramCodeLine(Type::Instruction), _address(address), _operation(operation), _operands(operands) { }

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

    void ResetVars();
    void FreeVars();

    void SetDefaultCodeStyle();

    //static void AppendStringFormat(std::string* str, const char* format, ...);

    bool m_bOk;
    CodeStyle m_codeStyle;
    CodeLineList m_codeLines;
    bool m_isDirty;
};
