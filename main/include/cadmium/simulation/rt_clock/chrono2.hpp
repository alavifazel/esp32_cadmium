/**
 * Real-time clock based on the chrono standard library.
 * Copyright (C) 2023  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP

#include <iostream>
#include <chrono>
#include <optional>
#include <thread>
#include "rt_clock.hpp"
#include "../../exception.hpp"
#include "time.h"

namespace cadmium {

    template <typename T = std::chrono::steady_clock>
    class ChronoClock: RealTimeClock {
     protected:
        time_t now;
        struct tm timeinfo;

        //std::chrono::time_point<T> rTimeLast; //!< last real system time.
        //std::optional<typename T::duration>  maxJitter; //!< Maximum allowed delay jitter. This parameter is optional.
     public:

        //! The empty constructor does not check the accumulated delay jitter.
        ChronoClock(): RealTimeClock() {

            if (!getLocalTime(&timeinfo)) {
                //Serial.println("Failed to obtain time");
                return(0);
            }
            time(&now);
        }


        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            time(&now);
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {

        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        void waitUntil(double timeNext) override {
            delay(timeNext);
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP
