/*
 *  AvanceDB - an in-memory database similar to Apache CouchDB
 *  Copyright (C) 2015-2017 Ripcord Software
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "http_server_log.h"

#include <iostream>
#include <cstdio>

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

#include "termcolor/termcolor.hpp"

#include "set_thread_name.h"
#include "config.h"

static const unsigned cumulativeSecsPerMonth[12] = { 2678400, 5097600, 7776000, 10368000, 13046400, 15638400, 18316800, 20995200, 23587200, 26265600, 28857600, 31536000 };
static const unsigned cumulativeSecsPerMonthLeap[12] = { 2678400, 5184000, 7862400, 10454400, 13132800, 15724800, 18403200, 21081600, 23673600, 26352000, 28944000, 31622400 };

struct LogRow {
    LogRow() : status(0) {
        data[0] = 0;
    }
    
    int status;
    char data[2048];
};

static bool colorEnabled = false;
static const unsigned maxLogRows = 4096;
static LogRow logRows[maxLogRows];
static boost::atomic<unsigned> writeRowIndex;
static unsigned readRowIndex = 0;

static boost::once_flag startLogging = BOOST_ONCE_INIT;

static void StreamWriterThread() {
    SetThreadName::Set("avancedb-httplog");
    
    try {
        while (true) {
            boost::this_thread::sleep_for(boost::chrono::seconds(2));
            
            int lastStatus = 200;
            auto lastRowIndex = writeRowIndex.load();
            
            while (readRowIndex < lastRowIndex) {
                for (auto i = readRowIndex; i < lastRowIndex; ++i) {
                    auto& row = logRows[i % maxLogRows];

                    if (colorEnabled && row.status != lastStatus) {
                        if (row.status >= 500) {
                            std::cout << termcolor::red;
                        } else if (row.status >= 400) {
                            std::cout << termcolor::grey << termcolor::bold;
                        } else if (row.status >= 300) {
                            std::cout << termcolor::yellow;
                        } else {
                            std::cout << termcolor::reset;
                        }

                        lastStatus = row.status;
                    }

                    std::cout << row.data << "\n";
                }

                readRowIndex = lastRowIndex;
                lastRowIndex = writeRowIndex.load();
            }                        
            
            if (colorEnabled) {
                std::cout << termcolor::reset;
            }
            
            std::cout << std::flush;
        }
    } catch (boost::thread_interrupted&) {
    }
}

static void StartLogging() {
    colorEnabled = Config::Process::Color();

    boost::thread logThread(StreamWriterThread);
    logThread.detach();
}

void HttpServerLog::Append(rs::httpserver::socket_ptr socket, rs::httpserver::request_ptr request, rs::httpserver::response_ptr response, const std::time_t& start, long duration) {
    // start the log thread if it isn't already running
    boost::call_once(StartLogging, startLogging);
    
    int year, month, day, hour, min, sec;
    GetTimestamp(start, year, month, day, hour, min, sec);
    
    const auto& queryString = request->getHeaders()->getQueryString();
    
    auto localAddr = socket->getLocalEndpoint().address().to_string();
    auto remoteAddr = socket->getRemoteEndpoint().address().to_string();

    auto userAgent = boost::replace_all_copy(request->getHeaders()->getUserAgent(), " ", "+");
    
    auto index = writeRowIndex++;    
    index %= maxLogRows;
    
    // TODO: optimize the string formatting to reduce ad-hoc allocations and re-use already known string values (dates, etc.)
    logRows[index].status = response->getStatusCode();
    std::snprintf(logRows[index].data, sizeof(logRows[0].data), 
        "%04d-%02d-%02d %02d:%02d:%02dZ %s %s %s %s %u %s %s %d %u",
        year, month, day, hour, min, sec,
        localAddr.c_str(),
        request->getMethod().c_str(),
        request->getUri().c_str(),
        queryString.size() > 0 ? queryString.c_str() : "-",
        socket->getLocalEndpoint().port(),
        remoteAddr.c_str(),
        userAgent.size() > 0 ? userAgent.c_str() : "-",
        response->getStatusCode(),
        duration);        
}

void HttpServerLog::GetTimestamp(std::time_t time, int& year, int& month, int& day, int& hour, int& min, int& sec) {
    decltype(time) offset = 0;
    auto isLeap = false;

    if (std::is_signed<decltype(time)>::value && time < 0) {
        time = 0;
    }
    
    if (time >= 1451606400) {
        GetYear(time, 2016, 1451606400, year, offset, isLeap);
    } else {
        GetYear(time, 1970, 0, year, offset, isLeap);
    }

    auto elapsedSecondsThisYear = time - offset;
    month = GetMonth(elapsedSecondsThisYear, isLeap);
    day = GetDay(elapsedSecondsThisYear, month, isLeap);
        
    ++month;
    ++day;
    
    hour = (time % (24 * 60 * 60)) / (60 * 60);
    min = (time % (60 * 60)) / 60;
    sec = time % 60;
}

void HttpServerLog::GetYear(std::time_t time, int startYear, std::time_t startOffset, int& year, std::time_t& offset, bool& isLeap) {
    isLeap = (startYear % 4) == 0;
    year = startYear;
    
    offset = startOffset;
    decltype(time) delta = isLeap ? 31622400 : 31536000;
    while ((offset + delta) <= time) {
        ++year;
        offset += delta;
        isLeap = (year % 4) == 0;
        delta = isLeap ? 31622400 : 31536000;
    }
}

unsigned HttpServerLog::GetMonth(long elapsedSeconds, bool isLeap) {
    const unsigned* lookup = isLeap ? cumulativeSecsPerMonthLeap : cumulativeSecsPerMonth;
    
    int i = 6, left = 0, right = 12;
    for (; left < right; i = left + ((right - left) / 2)) {
        if (elapsedSeconds < lookup[i]) {
            right = i - 1;
        } else {
            left = i + 1;
        }
    }
    
    return i;
}

unsigned HttpServerLog::GetDay(long elapsedSeconds, unsigned month, bool isLeap) {
    const unsigned* lookup = isLeap ? cumulativeSecsPerMonthLeap : cumulativeSecsPerMonth;
    
    auto dayMod = month > 0 ? elapsedSeconds - lookup[month - 1] : elapsedSeconds;
    auto day = dayMod / 86400;
    return day;
}