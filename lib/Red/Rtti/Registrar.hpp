#pragma once

namespace Red
{
class RTTIRegistrar
{
public:
    using Callback = void(*)();

    RTTIRegistrar(Callback aRegister, Callback aDescribe)
        : m_registered(false)
        , m_register(aRegister)
        , m_describe(aDescribe)
    {
        s_pending.push_back(this);
    }

    void Register()
    {
        if (!m_registered)
        {
            auto* rtti = CRTTISystem::Get();

            if (m_register)
                rtti->AddRegisterCallback(m_register);

            if (m_describe)
                rtti->AddPostRegisterCallback(m_describe);

            m_registered = true;
        }
    }

    static inline void RegisterPending()
    {
        for (const auto& pending : s_pending)
        {
            pending->Register();
        }

        s_pending.clear();
    }


private:
    bool m_registered;
    Callback m_register;
    Callback m_describe;

    static inline std::vector<RTTIRegistrar*> s_pending;
};
}
