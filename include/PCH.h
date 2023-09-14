#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Reshade/reshade.hpp"

#include <unordered_set>
#include <chrono>
#include <memory>
#include <Windows.h>
#include <cstdint>

using namespace std::literals;

template <typename T>
class Singleton
{
protected:
    constexpr Singleton() = default;
    constexpr ~Singleton() = default;

public:
    constexpr Singleton(const Singleton&) = delete;
    constexpr Singleton(Singleton&&) = delete;
    constexpr auto operator=(const Singleton&) = delete;
    constexpr auto operator=(Singleton&&) = delete;

    static T* GetSingleton()
    {
        static T singleton;
        return std::addressof(singleton);
    }
};

#if _DEBUG
	#define DEBUG_LOG(logger, msg, ...) logger->info(msg, __VA_ARGS__)
#else
	#define DEBUG_LOG(logger, msg, ...)
#endif