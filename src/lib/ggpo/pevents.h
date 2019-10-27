/*
 * WIN32 Events for POSIX
 * Author: Mahmoud Al-Qudsi <mqudsi@neosmart.net>
 * Copyright (C) 2011 - 2019 by NeoSmart Technologies
 * This code is released under the terms of the MIT License
 */

#pragma once

#if defined(_WIN32) && !defined(CreateEvent)
#error Must include Windows.h prior to including pevents.h!
#endif
#ifndef WAIT_TIMEOUT
#include <errno.h>
#define WAIT_TIMEOUT ETIMEDOUT
#endif

#include <stdint.h>

#define WFMO 1

namespace neosmart {
    // Type declarations
    struct neosmart_event_t_;
    typedef neosmart_event_t_ *neosmart_event_t;

    // Function declarations
    neosmart_event_t CreateEvent(bool manualReset = false, bool initialState = false);
    int DestroyEvent(neosmart_event_t event);
    int WaitForEvent(neosmart_event_t event, uint64_t milliseconds = -1);
    int SetEvent(neosmart_event_t event);
    int ResetEvent(neosmart_event_t event);
#ifdef WFMO
    int WaitForMultipleEvents(neosmart_event_t *events, int count, bool waitAll,
                              uint64_t milliseconds);
    int WaitForMultipleEvents(neosmart_event_t *events, int count, bool waitAll,
                              uint64_t milliseconds, int &index);
#endif
#ifdef PULSE
    int PulseEvent(neosmart_event_t event);
#endif
} // namespace neosmart