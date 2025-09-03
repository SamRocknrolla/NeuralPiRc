/*
  ==============================================================================

    ElapsedTimer.h
    Created: 17 Jun 2025 10:15:32pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ElapsedTimer
{
public:
    ElapsedTimer() = default;

    // Start the timer with a specific timeout (in milliseconds)
    void start(double timeoutMs)
    {
        const juce::SpinLock::ScopedLockType lock(lock_);
        startTimeMs = juce::Time::getMillisecondCounterHiRes();
        timeoutDurationMs = timeoutMs;
        valid = true;
    }

    // Returns true if the timeout duration has elapsed
    bool IsElapsed() const
    {
        const juce::SpinLock::ScopedLockType lock(lock_);
        if (!valid)
            return false;

        const double now = juce::Time::getMillisecondCounterHiRes();
        return (now - startTimeMs) >= timeoutDurationMs;
    }

    // Returns true if the timer was started
    bool isValid() const
    {
        const juce::SpinLock::ScopedLockType lock(lock_);
        return valid;
    }

    void stop() {
        valid = false;
    }

private:
    mutable juce::SpinLock lock_;

    double startTimeMs{ 0.0 };
    double timeoutDurationMs{ 0.0 };
    bool valid{ false };
};