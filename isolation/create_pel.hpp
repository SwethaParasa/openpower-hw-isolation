#pragma once

#include <phal_exception.H>

#include <nlohmann/json.hpp>
#include <xyz/openbmc_project/Logging/Create/server.hpp>
#include <xyz/openbmc_project/Logging/Entry/server.hpp>

#include <string>
#include <vector>
namespace openpower
{
namespace guard
{
namespace pel
{
using FFDCData = std::vector<std::pair<std::string, std::string>>;

using FFDCInfo = std::vector<std::tuple<
    sdbusplus::xyz::openbmc_project::Logging::server::Create::FFDCFormat,
    uint8_t, uint8_t, sdbusplus::message::unix_fd>>;

using Severity = sdbusplus::xyz::openbmc_project::Logging::server::Entry::Level;

using json = nlohmann::json;

using namespace openpower::phal;

/**
 * Create a PEL with FFDC file
 *
 * @brief This method creates a PEL based on the FFDC file provided.
 *
 * @param  event - the event type
 * @param  ffdcData - vector of additional ffdc data
 * @param  severity - the severity level
 * @param  ffcdInfo - a vector of tuples containing ffdc file info
 * @return Platform log id or 0 if error
 */
uint32_t createPelWithFFDCfiles(const std::string& event,
                                const FFDCData& ffdcData,
                                const Severity& severity,
                                const FFDCInfo& ffdcInfo);

/**
 * @brief Get DBUS service for input interface via mapper call
 *
 * @param[in] bus -  DBUS Bus Object
 * @param[in] intf - DBUS Interface
 * @param[in] path - DBUS Object Path
 *
 * @return distinct dbus name for input interface/path
 **/
std::string getService(sdbusplus::bus::bus& bus, const std::string& intf,
                       const std::string& path);

/**
 * @class FFDCFile
 * @brief This class is used to create ffdc data file and to get fd
 */
class FFDCFile
{
  public:
    FFDCFile() = delete;
    FFDCFile(const FFDCFile&) = delete;
    FFDCFile& operator=(const FFDCFile&) = delete;
    FFDCFile(FFDCFile&&) = delete;
    FFDCFile& operator=(FFDCFile&&) = delete;

    /**
     * Used to pass json object to create unique ffdc file by using
     * passed json data.
     */
    explicit FFDCFile(const json& pHALCalloutData);

    /**
     * Used to remove created ffdc file.
     */
    ~FFDCFile();

    /**
     * Used to get created ffdc file file descriptor id.
     *
     * @return file descriptor id
     */
    int getFileFD() const;

  private:
    /**
     * Used to store callout ffdc data from passed json object.
     */
    std::string calloutData;

    /**
     * Used to store unique ffdc file name.
     */
    std::string calloutFile;

    /**
     * Used to store created ffdc file descriptor id.
     */
    int fileFD;

    /**
     * Used to create ffdc file to pass PEL api for creating
     * pel records.
     *
     * @return NULL
     */
    void prepareFFDCFile();

    /**
     * Create unique ffdc file.
     *
     * @return NULL
     */
    void createCalloutFile();

    /**
     * Used write json object value into created file.
     *
     * @return NULL
     */
    void writeCalloutData();

    /**
     * Used set ffdc file seek position begining to consume by PEL
     *
     * @return NULL
     */
    void setCalloutFileSeekPos();

    /**
     * Used to remove created ffdc file.
     *
     * @return NULL
     */
    void removeCalloutFile();

}; // FFDCFile end

} // namespace pel
} // namespace guard
} // namespace openpower
