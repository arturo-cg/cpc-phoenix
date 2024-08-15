#include "stdafx.h"
#include "ProgramCode.h"

bool ProgramCode::Init()
{
    bool bRet = true;

    End();
    ResetVars();

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Initialize class members
    if (bRet)
    {

    }


    if (bRet)
    {
        m_bOk = true;
    }
    else
    {
        FreeVars();
    }

    return bRet;
}

/*virtual*/ void ProgramCode::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void ProgramCode::ResetVars()
{
    SetDefaultCodeStyle();
    m_codeLines.clear();
    m_isDirty = false;
}

void ProgramCode::FreeVars()
{
    Clear();
}

void ProgramCode::SetDefaultCodeStyle()
{
    m_codeStyle.addressWidth = 6;
    m_codeStyle.instructionOperationWidth = 5;
}

void ProgramCode::Clear()
{
    for (ProgramCodeLine* line : m_codeLines)
    {
        delete line;
    }

    m_codeLines.clear();
}

void ProgramCode::AddComment(const std::string& comment)
{
    ProgramCodeCommentLine* line = new ProgramCodeCommentLine(comment);
    m_codeLines.push_back(line);
}

void ProgramCode::AddDirective(const std::string& directive, const std::string& operands)
{
    ProgramCodeDirectiveLine* line = new ProgramCodeDirectiveLine(directive, operands);
    m_codeLines.push_back(line);
}

void ProgramCode::AddInstruction(cpcWord address, const std::string& operation, const std::string& operands)
{
    ProgramCodeInstructionLine* line = new ProgramCodeInstructionLine(address, operation, operands);
    m_codeLines.push_back(line);
}

void ProgramCode::DrawGui()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 50, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
    ImGui::BeginChild("Program code", ImVec2(0, -10), ImGuiChildFlags_Border);

    // Code lines.
    ImGuiListClipper clipper;
    clipper.Begin(m_codeLines.size());
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            ProgramCodeLine* line = m_codeLines[i];
            switch (line->_type)
            {
                case ProgramCodeLine::Type::Comment:
                {
                    ProgramCodeCommentLine* comment = (ProgramCodeCommentLine*)line;
                    ImGui::Text("%*c %s", m_codeStyle.addressWidth, ' ', comment->_comment.c_str());
                    break;
                }

                case ProgramCodeLine::Type::Directive:
                {
                    ProgramCodeDirectiveLine* directive = (ProgramCodeDirectiveLine*)line;
                    ImGui::Text("%*c %s %s", m_codeStyle.addressWidth, ' ', directive->_directive.c_str(), directive->_operands.c_str());
                    break;
                }

                case ProgramCodeLine::Type::Instruction:
                {
                    ProgramCodeInstructionLine* instruction = (ProgramCodeInstructionLine*)line;
                    ImGui::Text("%04X%*c %-*s %s", instruction->_address, m_codeStyle.addressWidth - 4, ' ', m_codeStyle.instructionOperationWidth, instruction->_operation.c_str(), instruction->_operands.c_str());
                    break;
                }

                default:
                {
                    KMASSERTM(false, ("Unhandled case."));
                    break;
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
}

//void ProgramCode::AppendStringFormat(std::string* str, const char* format, ...)
//{
//    va_list argList;
//    va_start(argList, format);
//
//    // Format the string and store the result in 'buffer'.
//    static char buffer[1000];
//    vsprintf_s(buffer, sizeof(buffer), format, argList);
//    // Append to 'str'.
//    str->append(buffer);
//
//    va_end(argList);
//}
