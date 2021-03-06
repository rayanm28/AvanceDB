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

#include "config.h"

#include <algorithm>
#include <cmath>

#include <boost/thread.hpp>

const char Config::Http::DefaultAddress[] = "0.0.0.0";
const unsigned Config::Http::DefaultPort = 5994;
const unsigned Config::Http::DefaultWorkersPerCpu = 16;
std::string Config::Http::address_ = DefaultAddress;
unsigned Config::Http::port_ = DefaultPort;
unsigned Config::Http::workersPerCpu_ = DefaultWorkersPerCpu;

const float Config::MapReduce::DefaultWorkersPerCpu = 0.5;
float Config::MapReduce::workersPerCpu_ = DefaultWorkersPerCpu;

unsigned Config::Environment::cpuCount_ = Config::Environment::RealCpuCount();

std::string Config::Process::pidFile_;
std::string Config::Process::stdOutFile_;
std::string Config::Process::stdErrFile_;
std::string Config::Process::rootDirectory_;

const std::uint32_t Config::SpiderMonkey::DefaultHeapSizeMB = 64;
const std::uint32_t Config::SpiderMonkey::DefaultNurserySizeMB = 16;
std::uint32_t Config::SpiderMonkey::heapSizeMB_ = DefaultHeapSizeMB;
std::uint32_t Config::SpiderMonkey::nurserySizeMB_ = DefaultNurserySizeMB;

static const char* processDaemon = "daemon";
static const char* processColor = "color";

static const char* envCpus = "cpus";

static const char* jsapiDisableBaseLineArg = "jsapi-disable-baseline";
static const char* jsapiDisableIonArg = "jsapi-disable-ion";

boost::program_options::options_description Config::desc_("Program options");
boost::program_options::variables_map Config::vm_;

void Config::Parse(int argc, const char** argv) {
    if (desc_.options().size() == 0) {
        desc_.add_options()
            ("help,h", "shows the program options")
            ("address,a", boost::program_options::value(&Http::address_)->default_value(Http::address_), "the IP address to listen on")
            ("port,p", boost::program_options::value(&Http::port_)->default_value(Http::port_), "the TCP/IP port to listen on")
            ("http-workers", boost::program_options::value(&Http::workersPerCpu_)->default_value(Http::workersPerCpu_), "the number of HTTP workers per CPU core")
            (processDaemon, "daemonize the process")
            (envCpus, boost::program_options::value(&Environment::cpuCount_)->default_value(Environment::cpuCount_), "set the number of CPUs to use")
            ("pid", boost::program_options::value(&Process::pidFile_), "writes the process id to a file")
            ("out,o", boost::program_options::value(&Process::stdOutFile_), "writes stdout to a file")
            ("err,e", boost::program_options::value(&Process::stdErrFile_), "writes stderr to a file")
            (processColor, "use color in the log file")
            ("dir", boost::program_options::value(&Process::rootDirectory_), "sets the working directory")
            ("mapreduce-workers", boost::program_options::value(&MapReduce::workersPerCpu_)->default_value(MapReduce::workersPerCpu_), "the number of map/reduce worker threads per CPU core")
            ("jsapi-heap-size", boost::program_options::value(&SpiderMonkey::heapSizeMB_)->default_value(SpiderMonkey::heapSizeMB_), "the JSAPI heap size in MB")
            ("jsapi-nursery-size", boost::program_options::value(&SpiderMonkey::nurserySizeMB_)->default_value(SpiderMonkey::nurserySizeMB_), "the JSAPI nursery size in MB")
            (jsapiDisableBaseLineArg, "disable the JSAPI baseline compiler")
            (jsapiDisableIonArg, "disable the JSAPI IonMonkey compiler")
        ;
    }

    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc_), vm_);
    boost::program_options::notify(vm_);
}

const boost::program_options::options_description& Config::Description() noexcept {
    return desc_;
}

bool Config::IsHelp() noexcept {
    return vm_.count("help") > 0;
}

bool Config::IsDaemon() noexcept {
    return Process::Daemonize();
}

const std::string& Config::Process::PidFile() noexcept {
    return pidFile_;
}

const std::string& Config::Process::StdOutFile() noexcept {
    return stdOutFile_;
}

const std::string& Config::Process::StdErrFile() noexcept {
    return stdErrFile_;
}

bool Config::Process::Color() noexcept {
    return vm_.count(processColor) > 0;
}

const std::string& Config::Process::RootDirectory() noexcept {
    return rootDirectory_;
}

bool Config::Process::Daemonize() noexcept {
    return vm_.count(processDaemon) > 0;
}

unsigned Config::Environment::CpuCount() {
    if (vm_.count(envCpus) > 0) {
        return cpuCount_;
    } else {
        return RealCpuCount();
    }
}

unsigned Config::Environment::RealCpuCount() {
    auto cores = std::max(1u, boost::thread::hardware_concurrency());
    return cores;
}

const std::string& Config::Http::Address() noexcept {
    return address_;
}

unsigned Config::Http::Port() noexcept {
    return port_;
}

unsigned Config::Http::Workers() noexcept {
    return WorkersPerCpu() * Environment::CpuCount();
}

unsigned Config::Http::WorkersPerCpu() noexcept {
    return workersPerCpu_;
}

std::uint32_t Config::SpiderMonkey::HeapSize() noexcept {
    return heapSizeMB_ * 1024 * 1024;
}

std::uint32_t Config::SpiderMonkey::NurserySize() noexcept {
    return nurserySizeMB_ * 1024 * 1024;
}

bool Config::SpiderMonkey::EnableBaselineCompiler() noexcept {
    return vm_.count(jsapiDisableBaseLineArg) == 0;
}

bool Config::SpiderMonkey::EnableIonCompiler() noexcept {
    return vm_.count(jsapiDisableIonArg) == 0;
}

unsigned Config::MapReduce::Workers() noexcept {
    auto workers = std::lround(WorkersPerCpu() * Environment::CpuCount());
    return std::max(1l, workers);
}

float Config::MapReduce::WorkersPerCpu() noexcept {
    return workersPerCpu_;
}

unsigned Config::Data::DatabaseDeleteDelay() noexcept {
    return 5;
}