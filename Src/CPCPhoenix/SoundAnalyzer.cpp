//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SoundAnalyzer.h"
#include "Application.h"

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
        ImVec2 graphSize = ImVec2(ImGui::GetContentRegionAvail().x - 100.f, ImGui::GetContentRegionAvail().y / 4.f);
        ImGui::PlotLines("Mixed", m_samplesMixed, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);
        ImGui::PlotLines("Channel A", m_samplesChannelA, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);
        ImGui::PlotLines("Channel B", m_samplesChannelB, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);
        ImGui::PlotLines("Channel C", m_samplesChannelC, NumSamples, m_nextPosition, nullptr/*overlay_text*/, -1.f, 1.f, graphSize, 4/*stride*/);
    }
    ImGui::End();

    if (!keepOpen)
    {
        SetActive(false);
    }
}
