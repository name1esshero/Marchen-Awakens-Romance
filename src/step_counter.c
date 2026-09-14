/* Two-phase pulse counter. The first update
 * pulses immediately. It then uses an initial interval for a limited number
 * of pulses before switching to a repeating interval. No animation-specific
 * interpretation is assumed. */
#include "gba/types.h"
#include "rom_section.h"

struct StepCounter
{
    s16 active;
    s16 initialInterval;
    s16 initialPulseLimit;
    s16 repeatInterval;
    s16 repeatTicks;
    s16 initialTicks;
    s16 initialPulses;
};

AT("00053960")
void StepCounterInit(struct StepCounter *counter, u16 first, u16 limit, u16 step)
{
    counter->initialInterval = first;
    counter->initialPulseLimit = limit;
    counter->repeatInterval = step;
    counter->repeatTicks = 0;
    counter->active = 0;
    counter->initialTicks = 0;
    counter->initialPulses = 0;
}

AT("00053978")
void StepCounterReset(struct StepCounter *counter)
{
    counter->repeatTicks = 0;
    counter->active = 0;
    counter->initialTicks = 0;
    counter->initialPulses = 0;
}

/* Counter increments truncate to signed 16 bits before comparison. */
AT("00053984")
u32 StepCounterAdvance(struct StepCounter *counter)
{
    u32 pulse = 0;
    if (counter->active)
    {
        if (counter->initialPulses >= counter->initialPulseLimit)
        {
            if (++counter->repeatTicks >= counter->repeatInterval)
            {
                counter->repeatTicks = 0;
                pulse = 1;
            }
        }
        else
        {
            if (++counter->initialTicks >= counter->initialInterval)
            {
                counter->initialTicks = 0;
                counter->initialPulses++;
                pulse = 1;
            }
        }
    }
    else
    {
        pulse = 1;
        counter->active = pulse;
    }
    return pulse;
}
