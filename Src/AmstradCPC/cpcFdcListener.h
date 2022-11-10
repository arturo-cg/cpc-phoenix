#pragma once

namespace CPC {

    class CFdc;

    class IFdcListener
    {
    public:
        /** Called after receiving a command and all its parameters, right before starting the execution phase. */
        virtual void OnFdcCommandReceived(const CFdc* fdc) = 0;
        /** Called after finishing executing a command and sending all its results to the CPU. */
        virtual void OnFdcCommandFinished(const CFdc* fdc) = 0;
    };

}
