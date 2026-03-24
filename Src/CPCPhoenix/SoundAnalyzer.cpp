//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SoundAnalyzer.h"
#include "Application.h"
#include "cpcSoundOutput.h"

bool SoundAnalyzer::Init()
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
        //...
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

/*virtual*/ void SoundAnalyzer::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void SoundAnalyzer::ResetVars()
{
    m_active = false;
    ClearBuffers();
    m_nextPosition = 0;
}

void SoundAnalyzer::FreeVars()
{
    //...
}

void SoundAnalyzer::SetActive(bool active)
{
    m_active = active;

    if (active)
    {
        ClearBuffers();
        m_nextPosition = 0;
        Application::Singleton()->GetSoundOutput()->SetListener(this);
    }
    else
    {
        Application::Singleton()->GetSoundOutput()->SetListener(nullptr);
    }
}

void SoundAnalyzer::ClearBuffers()
{
    for (unsigned i = 0; i < NumSamples; i++)
    {
        m_samplesMixed[i] = 0.f;
        m_samplesChannelA[i] = 0.f;
        m_samplesChannelB[i] = 0.f;
        m_samplesChannelC[i] = 0.f;
    }
}

void SoundAnalyzer::OnNewSoundSample(float sampleMixed, float sampleChannelA, float sampleChannelB, float sampleChannelC)
{
    m_samplesMixed[m_nextPosition] = sampleMixed;
    m_samplesChannelA[m_nextPosition] = sampleChannelA;
    m_samplesChannelB[m_nextPosition] = sampleChannelB;
    m_samplesChannelC[m_nextPosition] = sampleChannelC;
    m_nextPosition = (m_nextPosition + 1) % NumSamples;
}

void SoundAnalyzer::DrawGui()
{
    bool keepOpen = IsActive();
    if (ImGui::Begin("SoundAnalyzer", &keepOpen/*, ImGuiWindowFlags_AlwaysAutoResize*/))
    {
        // Mixed samples.
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x);
        ImVec2 graphSize = ImVec2(ImGui::GetContentRegionAvail().x - 100.f, ImGui::GetContentRegionAvail().y / 4.f);
        ImGui::PlotLines("Mixed", m_samplesMixed, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);
        // Channels.
        DrawChannel("Channel A", 0, m_samplesChannelA, graphSize);
        DrawChannel("Channel B", 1, m_samplesChannelB, graphSize);
        DrawChannel("Channel C", 2, m_samplesChannelC, graphSize);
    }
    ImGui::End();

    if (!keepOpen)
    {
        SetActive(false);
    }
}

void SoundAnalyzer::DrawChannel(const char* label, int channelIndex, float* samples, ImVec2 graphSize)
{
    CPC::CSoundOutput* soundOutput = Application::Singleton()->GetEmulatedMachine()->GetSoundOutput();
    bool channelEnabled = soundOutput->IsChannelEnabled(channelIndex);
    std::string checkboxId = std::string("##Enable_") + std::string(label);
    ImGui::Checkbox(checkboxId.c_str(), &channelEnabled);
    ImGui::SameLine();
    ImGui::PlotLines(label, samples, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);

    if (channelEnabled != soundOutput->IsChannelEnabled(channelIndex))
    {
        soundOutput->SetChannelEnabled(channelIndex, channelEnabled);
    }
}
