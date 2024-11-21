#include "create_pel.hpp"

#include <fcntl.h>
#include <libekb.H>
#include <phal_exception.H>
#include <unistd.h>

#include <phosphor-logging/elog.hpp>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
namespace openpower
{
using namespace phosphor::logging;

namespace dump
{
namespace pel
{

constexpr auto loggingObjectPath = "/xyz/openbmc_project/logging";
constexpr auto loggingInterface = "xyz.openbmc_project.Logging.Create";
constexpr auto opLoggingInterface = "org.open_power.Logging.PEL";

using Level = sdbusplus::xyz::openbmc_project::Logging::server::Entry::Level;

std::string getService(sdbusplus::bus::bus& bus, const std::string& intf,
                       const std::string& path)
{
    constexpr auto MAPPER_BUSNAME = "xyz.openbmc_project.ObjectMapper";
    constexpr auto MAPPER_PATH = "/xyz/openbmc_project/object_mapper";
    constexpr auto MAPPER_INTERFACE = "xyz.openbmc_project.ObjectMapper";
    try
    {
        auto mapper = bus.new_method_call(MAPPER_BUSNAME, MAPPER_PATH,
                                          MAPPER_INTERFACE, "GetObject");

        mapper.append(path, std::vector<std::string>({intf}));

        auto mapperResponseMsg = bus.call(mapper);
        std::map<std::string, std::vector<std::string>> mapperResponse;
        mapperResponseMsg.read(mapperResponse);

        if (mapperResponse.empty())
        {
            log<level::ERR>(std::format("Empty mapper response for GetObject "
                                        "interface({}), path({})",
                                        intf, path)
                                .c_str());
            throw std::runtime_error("Empty mapper response for GetObject");
        }
        return mapperResponse.begin()->first;
    }
    catch (const sdbusplus::exception::exception& ex)
    {
        log<level::ERR>(std::format("Mapper call failed for GetObject "
                                    "errorMsg({}), path({}), interface({}) ",
                                    ex.what(), path, intf)
                            .c_str());
        throw;
    }
}

uint32_t createPelWithFFDCfiles(const std::string& event,
                                const FFDCData& ffdcData,
                                const Severity& severity,
                                const FFDCInfo& ffdcInfo)
{
    uint32_t plid = 0;
    try
    {
        auto bus = sdbusplus::bus::new_default();

        std::unordered_map<std::string, std::string> additionalData;
        for (auto& data : ffdcData)
        {
            additionalData.emplace(data);
        }

        auto service = getService(bus, opLoggingInterface,
                                        loggingObjectPath);
        auto method = bus.new_method_call(service.c_str(), loggingObjectPath,
                                          opLoggingInterface,
                                          "CreatePELWithFFDCFiles");
        auto level =
            sdbusplus::xyz::openbmc_project::Logging::server::convertForMessage(
                severity);
        method.append(event, level, additionalData, ffdcInfo);
        auto response = bus.call(method);

        // reply will be tuple containing bmc log id, platform log id
        std::tuple<uint32_t, uint32_t> reply = {0, 0};

        // parse dbus response into reply
        response.read(reply);
        plid = std::get<1>(reply); // platform log id is tuple "second"
    }
    catch (const sdbusplus::exception::exception& e)
    {
        log<level::ERR>(std::format("D-Bus call exception",
                                    "OBJPATH={}, INTERFACE={}, EXCEPTION={}",
                                    loggingObjectPath, loggingInterface,
                                    e.what())
                            .c_str());
        throw;
    }
    catch (const std::exception& e)
    {
        throw;
    }
    return plid;
}

FFDCFile::FFDCFile(const json& pHALCalloutData) :
    calloutData(pHALCalloutData.dump()),
    calloutFile("/tmp/phalPELCalloutsJson.XXXXXX"), fileFD(-1)
{
    prepareFFDCFile();
}

FFDCFile::~FFDCFile()
{
    removeCalloutFile();
}

int FFDCFile::getFileFD() const
{
    return fileFD;
}

void FFDCFile::prepareFFDCFile()
{
    createCalloutFile();
    writeCalloutData();
    setCalloutFileSeekPos();
}

void FFDCFile::createCalloutFile()
{
    fileFD = mkostemp(const_cast<char*>(calloutFile.c_str()), O_RDWR);

    if (fileFD == -1)
    {
        log<level::ERR>(std::format("Failed to create phalPELCallouts "
                                    "file({}), errorno({}) and errormsg({})",
                                    calloutFile, errno, strerror(errno))
                            .c_str());
        throw std::runtime_error("Failed to create phalPELCallouts file");
    }
}

void FFDCFile::writeCalloutData()
{
    ssize_t rc = write(fileFD, calloutData.c_str(), calloutData.size());

    if (rc == -1)
    {
        log<level::ERR>(std::format("Failed to write phaPELCallout info "
                                    "in file({}), errorno({}), errormsg({})",
                                    calloutFile, errno, strerror(errno))
                            .c_str());
        throw std::runtime_error("Failed to write phalPELCallouts info");
    }
    else if (rc != static_cast<ssize_t>(calloutData.size()))
    {
        log<level::WARNING>(std::format("Could not write all phal callout "
                                        "info in file({}), written byte({}) "
                                        "and total byte({})",
                                        calloutFile, rc, calloutData.size())
                                .c_str());
    }
}

void FFDCFile::setCalloutFileSeekPos()
{
    int rc = lseek(fileFD, 0, SEEK_SET);

    if (rc == -1)
    {
        log<level::ERR>(std::format("Failed to set SEEK_SET for "
                                    "phalPELCallouts in file({}), errorno({}) "
                                    "and errormsg({})",
                                    calloutFile, errno, strerror(errno))
                            .c_str());
        throw std::runtime_error(
            "Failed to set SEEK_SET for phalPELCallouts file");
    }
}

void FFDCFile::removeCalloutFile()
{
    close(fileFD);
    std::remove(calloutFile.c_str());
}

} // namespace pel
} // namespace dump
} // namespace openpower
