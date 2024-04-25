// SPDX-License-Identifier: Apache-2.0

#include "common/isolatable_hardwares.hpp"

#include "common/utils.hpp"

#include <attributes_info.H>

#include <phosphor-logging/elog-errors.hpp>

#include <format>

namespace hw_isolation
{
using namespace phosphor::logging;

namespace isolatable_hws
{

constexpr auto CommonInventoryItemIface = "xyz.openbmc_project.Inventory.Item";

IsolatableHWs::IsolatableHWs(sdbusplus::bus::bus& bus) : _bus(bus)
{
    /**
     * @brief HwId consists with below ids.
     *
     * 1 - The inventory item interface name
     * 2 - The pdbg class name
     */
    // The below HwIds will be used to many units as parent fru
    // so creating one object which can reuse.
    IsolatableHWs::HW_Details::HwId processorHwId(
        "xyz.openbmc_project.Inventory.Item.Cpu", "proc");
    IsolatableHWs::HW_Details::HwId dimmHwId(
        "xyz.openbmc_project.Inventory.Item.Dimm", "dimm");
    IsolatableHWs::HW_Details::HwId emptyHwId("", "");
    bool ItIsFRU = true;

    _isolatableHWsList = {
        // FRU (Field Replaceable Unit) which are present in
        // OpenPOWER based system

        {processorHwId, IsolatableHWs::HW_Details(
                            ItIsFRU, emptyHwId, devtree::lookup_func::mruId,
                            inv_path_lookup_func::itemInstanceId, "")},

        {dimmHwId, IsolatableHWs::HW_Details(
                       ItIsFRU, emptyHwId, devtree::lookup_func::locationCode,
                       inv_path_lookup_func::itemLocationCode, "")},

        {IsolatableHWs::HW_Details::HwId(
             "xyz.openbmc_project.Inventory.Item.Tpm", "tpm"),
         IsolatableHWs::HW_Details(ItIsFRU, emptyHwId,
                                   devtree::lookup_func::locationCode,
                                   inv_path_lookup_func::itemLocationCode, "")},

        // Processor Subunits

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "eq"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "Quad")},

        // In BMC inventory, Core and FC representing as
        // "Inventory.Item.CpuCore" since both are core and it will model based
        // on the system core mode.
        {IsolatableHWs::HW_Details::HwId(
             "xyz.openbmc_project.Inventory.Item.CpuCore", "fc"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::pdbgIndex,
                                   inv_path_lookup_func::itemInstanceId, "")},

        {IsolatableHWs::HW_Details::HwId(
             "xyz.openbmc_project.Inventory.Item.CpuCore", "core"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemInstanceId, "")},

        // In BMC inventory, ECO mode core is modeled as a subunit since it
        // is not the normal core
        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "core"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "Cache-Only Core")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "mc"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "Memory Controller")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "mi"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Processor To Memory Buffer Interface")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "mcc"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Memory Controller Channel")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "omi"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "OpenCAPI Memory Interface")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "pauc"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "POWER Accelerator Unit Controller")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "pau"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "POWER Accelerator Unit")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "omic"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "OpenCAPI Memory Interface Controller")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "iohs"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "High speed SMP/OpenCAPI Link")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "smpgroup"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "OBUS End Point")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "pec"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "PCI Express controllers")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "phb"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::chipUnitPos,
             inv_path_lookup_func::itemPrettyName, "PCIe host bridge (PHB)")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "nmmu"),
         IsolatableHWs::HW_Details(!ItIsFRU, processorHwId,
                                   devtree::lookup_func::chipUnitPos,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Nest Memory Management Unit")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "nx"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, processorHwId, devtree::lookup_func::mruId,
             inv_path_lookup_func::itemPrettyName, "Accelerator")},

        // Memory (aka DIMM) subunits

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "ocmb"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, dimmHwId, devtree::lookup_func::pdbgIndex,
             inv_path_lookup_func::itemPrettyName, "OpenCAPI Memory Buffer")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "mem_port"),
         IsolatableHWs::HW_Details(
             !ItIsFRU, dimmHwId, devtree::lookup_func::pdbgIndex,
             inv_path_lookup_func::itemPrettyName, "DDR Memory Port")},

        // ADC and GPIO Expander are Generic I2C Device
        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "adc"),
         IsolatableHWs::HW_Details(!ItIsFRU, dimmHwId,
                                   devtree::lookup_func::pdbgIndex,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Onboard Memory Power Control Device")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface,
                                         "gpio_expander"),
         IsolatableHWs::HW_Details(!ItIsFRU, dimmHwId,
                                   devtree::lookup_func::pdbgIndex,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Onboard Memory Power Control Device")},

        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "pmic"),
         IsolatableHWs::HW_Details(!ItIsFRU, dimmHwId,
                                   devtree::lookup_func::pdbgIndex,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Onboard Memory Power Management IC")},

        // Motherboard subunits

        /**
         * The oscrefclk parent fru is not modelled in the phal cec device tree
         * so using the temporary workaround (refer getClkParentFruObjPath())
         * instead of defining the isolatable hardwares list.
         */
        {IsolatableHWs::HW_Details::HwId(CommonInventoryItemIface, "oscrefclk"),
         IsolatableHWs::HW_Details(!ItIsFRU, emptyHwId,
                                   devtree::lookup_func::pdbgIndex,
                                   inv_path_lookup_func::itemPrettyName,
                                   "Oscillator Reference Clock")},
    };
}

std::optional<
    std::pair<IsolatableHWs::HW_Details::HwId, IsolatableHWs::HW_Details>>
    IsolatableHWs::getIsotableHWDetails(
        const IsolatableHWs::HW_Details::HwId& id) const
{
    auto it = std::find_if(_isolatableHWsList.begin(), _isolatableHWsList.end(),
                           [&id](const auto& isolatableHw) {
        return isolatableHw.first == id;
    });

    if (it != _isolatableHWsList.end())
    {
        return *it;
    }
    return std::nullopt;
}

std::optional<
    std::pair<IsolatableHWs::HW_Details::HwId, IsolatableHWs::HW_Details>>
    IsolatableHWs::getIsolatableHWDetailsByPrettyName(
        const std::string& prettyName) const
{
    auto it = std::find_if(_isolatableHWsList.begin(), _isolatableHWsList.end(),
                           [&prettyName](const auto& isolatableHw) {
        return isolatableHw.second._prettyName == prettyName;
    });

    if (it != _isolatableHWsList.end())
    {
        return *it;
    }
    return std::nullopt;
}

std::optional<
    std::pair<IsolatableHWs::HW_Details::HwId, IsolatableHWs::HW_Details>>
    IsolatableHWs::getIsotableHWDetailsByObjPath(
        const sdbusplus::message::object_path& dbusObjPath) const
{
    std::map<std::string, std::vector<std::string>> objServs;

    try
    {
        auto method = _bus.new_method_call(type::ObjectMapperName,
                                           type::ObjectMapperPath,
                                           type::ObjectMapperName, "GetObject");

        method.append(dbusObjPath.str);
        method.append(std::vector<std::string>({}));

        auto reply = _bus.call(method);
        reply.read(objServs);
    }
    catch (const sdbusplus::exception::exception& e)
    {
        log<level::ERR>(std::format("Exception [{}] to get the given object "
                                    "[{}] interfaces",
                                    e.what(), dbusObjPath.str)
                            .c_str());
        return std::nullopt;
    }

    /**
     * Get only inventory item interface alone so we can minimize
     * iteration to get isolatable hardware details.
     *
     * Note, "xyz.openbmc_project.Inventory.Item" is generic interface
     * so no need to look for that.
     */
    std::vector<std::pair<std::string, std::string>> inventoryItemIfaces;
    std::for_each(objServs.begin(), objServs.end(),
                  [&inventoryItemIfaces](const auto& service) {
        auto inventoryItemIfaceIt = std::find_if(service.second.begin(),
                                                 service.second.end(),
                                                 [](const auto& interface) {
            return ((interface.find("Inventory.Item") != std::string::npos) &&
                    (!interface.ends_with("Item")));
        });

        if (inventoryItemIfaceIt != service.second.end())
        {
            inventoryItemIfaces.emplace_back(
                std::make_pair(service.first, *inventoryItemIfaceIt));
        }
    });

    if (inventoryItemIfaces.empty())
    {
        log<level::ERR>(std::format("The given object [{}] does not contains "
                                    "any inventory item interface",
                                    dbusObjPath.str)
                            .c_str());
        return std::nullopt;
    }
    else if (inventoryItemIfaces.size() > 1)
    {
        /**
         * FIXME: Assumption is, the OpenBMC project does not allow to host
         *        the same interface by the different services, and more than
         *        one different inventory item interface (since those will be
         *        achieved by the Association) in the same object.
         */
        std::stringstream objData;
        std::for_each(inventoryItemIfaces.begin(), inventoryItemIfaces.end(),
                      [&objData](const auto& ele) {
            objData << "Service: " << ele.first << " Iface: " << ele.second
                    << " | ";
        });

        log<level::ERR>(
            std::format("Either the same interface is hosted "
                        "by different services or different inventory item "
                        "interfaces are hosted in the same object [{}]. "
                        "ObjectData [{}]",
                        dbusObjPath.str, objData.str())
                .c_str());
        return std::nullopt;
    }

    auto objHwId{IsolatableHWs::HW_Details::HwId{
        IsolatableHWs::HW_Details::HwId::ItemInterfaceName(
            inventoryItemIfaces[0].second)}};

    // TODO Below decision need to be based on system core mode
    //     i.e whether need to use "fc" (in big core system) or
    //     "core" (in small core system) pdbg target class to get
    //     the appropriate isotable hardware details.
    if (objHwId._interfaceName._name.ends_with("CpuCore"))
    {
        objHwId = IsolatableHWs::HW_Details::HwId{
            IsolatableHWs::HW_Details::HwId::PhalPdbgClassName("fc")};
    }

    return getIsotableHWDetails(objHwId);
}

LocationCode IsolatableHWs::getLocationCode(
    const sdbusplus::message::object_path& dbusObjPath)
{
    return utils::getDBusPropertyVal<LocationCode>(
        _bus, dbusObjPath,
        "xyz.openbmc_project.Inventory.Decorator.LocationCode", "LocationCode");
}

std::optional<sdbusplus::message::object_path>
    IsolatableHWs::getParentFruObjPath(
        const sdbusplus::message::object_path& isolateHardware,
        const IsolatableHWs::HW_Details::HwId::ItemInterfaceName&
            parentFruIfaceName) const
{
    std::map<std::string, std::map<std::string, std::vector<std::string>>>
        parentObjs;

    try
    {
        auto method =
            _bus.new_method_call(type::ObjectMapperName, type::ObjectMapperPath,
                                 type::ObjectMapperName, "GetAncestors");

        method.append(isolateHardware.str);
        method.append(std::vector<std::string>({parentFruIfaceName._name}));

        auto reply = _bus.call(method);
        reply.read(parentObjs);
    }
    catch (const sdbusplus::exception::exception& e)
    {
        log<level::ERR>(
            std::format("Exception [{}] to get the given object [{}] parent "
                        "by using the given parent interface [{}]",
                        e.what(), isolateHardware.str, parentFruIfaceName._name)
                .c_str());
        return std::nullopt;
    }

    if (parentObjs.empty())
    {
        log<level::ERR>(
            std::format("The given object [{}] does not contain any parent "
                        "with the given parent interface [{}]",
                        isolateHardware.str, parentFruIfaceName._name)
                .c_str());
        return std::nullopt;
    }
    else if (parentObjs.size() > 1)
    {
        // Should not happen, Always we will have one parent object with
        // the given parent interface for the given child object.
        log<level::ERR>(
            std::format("The given object [{}] contain more than one parent "
                        "with the given parent interface [{}]",
                        isolateHardware.str, parentFruIfaceName._name)
                .c_str());
        return std::nullopt;
    }

    return parentObjs.begin()->first;
}

std::optional<devtree::DevTreePhysPath> IsolatableHWs::getPhysicalPath(
    const sdbusplus::message::object_path& isolateHardware)
{
    try
    {
        // Currently the subunit (unitN) is not modeled in the inventory
        // so we cannot locate the right subunit in the CEC device tree.
        if (isolateHardware.filename().starts_with("unit"))
        {
            log<level::ERR>(
                std::format("Not allowed to isolate the given hardware [{}] "
                            "which is not modeled in BMC inventory",
                            isolateHardware.str)
                    .c_str());
            return std::nullopt;
        }

        auto isolateHwDetails = getIsotableHWDetailsByObjPath(isolateHardware);
        if (!isolateHwDetails.has_value())
        {
            log<level::ERR>(
                std::format("The given hardware inventory object [{}] "
                            "item interface is not found in isolatable "
                            "hardware list",
                            isolateHardware.str)
                    .c_str());
            return std::nullopt;
        }

        // Make sure the given isolateHardware inventory path is exist
        // getDBusServiceName() will throw exception if the given object
        // is not exist.
        utils::getDBusServiceName(_bus, isolateHardware.str,
                                  isolateHwDetails->first._interfaceName._name);

        auto isolateHwInstanceId =
            utils::getInstanceId(isolateHardware.filename());
        if (!isolateHwInstanceId.has_value())
        {
            return std::nullopt;
        }

        struct pdbg_target* isolateHwTarget;
        devtree::lookup_func::CanGetPhysPath canGetPhysPath{false};

        if (isolateHwDetails->second._isItFRU)
        {
            auto unExpandedLocCode{devtree::getUnexpandedLocCode(
                getLocationCode(isolateHardware))};
            if (!unExpandedLocCode.has_value())
            {
                return std::nullopt;
            }

            pdbg_for_each_class_target(
                isolateHwDetails->first._pdbgClassName._name.c_str(),
                isolateHwTarget)
            {
                canGetPhysPath = isolateHwDetails->second._physPathFuncLookUp(
                    isolateHwTarget, *isolateHwInstanceId, *unExpandedLocCode);

                if (canGetPhysPath)
                {
                    // In scenarios where multiple logical DIMMs are installed,
                    // and the current DIMM is found to be absent, we must
                    // proceed to deconfigure the remaining dimm. So check for
                    // the current dimm's present status
                    ATTR_HWAS_STATE_Type hwasState;
                    if (!DT_GET_PROP(ATTR_HWAS_STATE, isolateHwTarget,
                                     hwasState))
                    {
                        // If not present, continue to find the other dimm that
                        // matches the location code
                        if (hwasState.present)
                        {
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            auto parentFruObjPath = getParentFruObjPath(
                isolateHardware,
                isolateHwDetails->second._parentFruHwId._interfaceName);
            if (!parentFruObjPath.has_value())
            {
                return std::nullopt;
            }

            auto parentFruInstanceId =
                utils::getInstanceId(parentFruObjPath->filename());
            if (!parentFruInstanceId.has_value())
            {
                return std::nullopt;
            }

            auto parentFruHwDetails =
                getIsotableHWDetails(isolateHwDetails->second._parentFruHwId);
            if (!parentFruHwDetails.has_value())
            {
                log<level::ERR>(
                    std::format("Parent fru details for the given isolate "
                                "hardware object name [{}] is not found in "
                                "isolatable hardware list",
                                isolateHardware.filename())
                        .c_str());
                return std::nullopt;
            }

            auto unExpandedLocCode{devtree::getUnexpandedLocCode(
                getLocationCode(*parentFruObjPath))};
            if (!unExpandedLocCode.has_value())
            {
                return std::nullopt;
            }

            struct pdbg_target* parentFruTarget;

            pdbg_for_each_class_target(
                parentFruHwDetails->first._pdbgClassName._name.c_str(),
                parentFruTarget)
            {
                canGetPhysPath = parentFruHwDetails->second._physPathFuncLookUp(
                    parentFruTarget, *parentFruInstanceId, *unExpandedLocCode);

                if (!canGetPhysPath)
                {
                    continue;
                }

                pdbg_for_each_target(
                    isolateHwDetails->first._pdbgClassName._name.c_str(),
                    parentFruTarget, isolateHwTarget)
                {
                    canGetPhysPath =
                        isolateHwDetails->second._physPathFuncLookUp(
                            isolateHwTarget, *isolateHwInstanceId,
                            *unExpandedLocCode);

                    if (canGetPhysPath)
                    {
                        break;
                    }
                }

                // No use to check other parent, since it will enter here
                // if identified the parent or,
                // if found the isolate hardware to get physical path or,
                // if not found the isolate hardware to get physical path
                if (canGetPhysPath)
                {
                    break;
                }
            }
        }

        if (!canGetPhysPath)
        {
            log<level::ERR>(std::format("Given hardware [{}] is not found "
                                        " in phal cec device tree",
                                        isolateHardware.str)
                                .c_str());
            return std::nullopt;
        }

        return devtree::getPhysicalPath(isolateHwTarget);
    }
    catch (const std::exception& e)
    {
        log<level::ERR>(std::format("Exception [{}]", e.what()).c_str());
        return std::nullopt;
    }
}

std::optional<std::vector<sdbusplus::message::object_path>>
    IsolatableHWs::getInventoryPathsByLocCode(
        const LocationCode& unexpandedLocCode)
{
    constexpr auto vpdMgrObjPath = "/com/ibm/VPD/Manager";
    constexpr auto vpdInterface = "com.ibm.VPD.Manager";

    std::vector<sdbusplus::message::object_path> listOfInventoryObjPaths;

    try
    {
        // FIXME: Use mapper to get dbus name instad of hardcode like below
        //        but, mapper failing when using "com.ibm.VPD" dbus tree.
        std::string dbusServiceName{"com.ibm.VPD.Manager"};

        auto method = _bus.new_method_call(dbusServiceName.c_str(),
                                           vpdMgrObjPath, vpdInterface,
                                           "GetFRUsByUnexpandedLocationCode");

        // passing 0 as node number
        // FIXME if enabled multi node system
        method.append(unexpandedLocCode, static_cast<uint16_t>(0));

        auto resp = _bus.call(method);

        resp.read(listOfInventoryObjPaths);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>(std::format("Exception [{}] to get inventory path for "
                                    "the given location code [{}]",
                                    e.what(), unexpandedLocCode)
                            .c_str());
        return std::nullopt;
    }

    return listOfInventoryObjPaths;
}

std::optional<struct pdbg_target*>
    IsolatableHWs::getParentFruPhalDevTreeTgt(struct pdbg_target* devTreeTgt)
{
    std::string fruUnitDevTreePath{pdbg_target_path(devTreeTgt)};
    std::string fruUnitPdbgClass{pdbg_target_class_name(devTreeTgt)};

    struct pdbg_target* parentFruTarget = nullptr;
    if ((fruUnitPdbgClass == "ocmb") || (fruUnitPdbgClass == "mem_port") ||
        (fruUnitPdbgClass == "adc") || (fruUnitPdbgClass == "gpio_expander") ||
        (fruUnitPdbgClass == "pmic"))
    {
        /**
         * FIXME: The assumption is, dimm is parent fru for "ocmb", "mem_port",
         *        "adc", "gpio_expander", and "pmic" units and those units
         *        will have only one "dimm" so if something is changed then,
         *        need to fix this logic.
         * @note  In phal cec device tree dimm is placed under ocmb->mem_port
         *        based on dimm pervasive path.
         */
        if ((fruUnitPdbgClass == "adc") ||
            (fruUnitPdbgClass == "gpio_expander") ||
            (fruUnitPdbgClass == "pmic") || (fruUnitPdbgClass == "mem_port"))
        {
            /**
             * The "adc", "gpio_expander", and "pmic" units are placed under
             * ocmb but, dimm is placed under the ocmb so, we need to get the
             * parent ocmb for the given "adc", "gpio_expander", and "pmic"
             * units to get the dimm fru target.
             */
            devTreeTgt = pdbg_target_parent("ocmb", devTreeTgt);
        }
	/** As ocmb has location code(which matches either dimm location code
 	 *  or the planar based on the type of the system), returning ocmb target
 	 *  for "adc", "pmic",  "gpio_expander", "mem_port" to support all the systems.
 	 */
	return devTreeTgt;
    }
    else
    {
        /**
         * FIXME: Today, All FRU parts (units - both(chiplet and non-chiplet))
         *        are modelled under the respective processor in cec device
         *        tree so, if something changed then, need to revisit the
         *        logic which is used to get the FRU details of FRU unit.
         */
        parentFruTarget = pdbg_target_parent("proc", devTreeTgt);
        if (parentFruTarget == nullptr)
        {
            log<level::ERR>(
                std::format("Failed to get the processor target from phal cec "
                            "device tree for the given target [{}]",
                            fruUnitDevTreePath)
                    .c_str());
            return std::nullopt;
        }
    }
    return parentFruTarget;
}

std::optional<sdbusplus::message::object_path>
    IsolatableHWs::getFRUInventoryPath(
        const std::pair<LocationCode, InstanceId>& fruDetails,
        const inv_path_lookup_func::LookupFuncForInvPath& fruInvPathLookupFunc)
{
    auto inventoryPathList = getInventoryPathsByLocCode(fruDetails.first);
    if (!inventoryPathList.has_value())
    {
        return std::nullopt;
    }

    if (inventoryPathList->empty())
    {
        // The inventory object doesn't exist for the given location code.
        log<level::ERR>(
            std::format("The inventory object does not exist for the given "
                        "location code [{}].",
                        fruDetails.first)
                .c_str());
        return std::nullopt;
    }
    else if (inventoryPathList->size() == 1)
    {
        // Only one inventory object is exist for the given location code.
        // For example, DIMM
        return (*inventoryPathList)[0];
    }
    else
    {
        /**
         * More than one inventory objects are exist for the given location code
         * so use the given instance id to get the right inventory object.
         *
         * For example, two processor in the Dual-Chip-Module will have the
         * same location code so the processor MRU_ID (aka instance id) is
         * included in the inventory object segment to get the right inventory
         * object.
         */
        inv_path_lookup_func::UniqueHwId fruInstId{fruDetails.second};

        auto fruHwInvPath = std::find_if(
            inventoryPathList->begin(), inventoryPathList->end(),
            [&fruInstId, &fruInvPathLookupFunc, this](const auto& path) {
            return fruInvPathLookupFunc(this->_bus, path, fruInstId);
            });

        if (fruHwInvPath == inventoryPathList->end())
        {
            log<level::ERR>(
                std::format("The inventory object does not exist for "
                            "the given location code [{}] and instance id [{}]",
                            fruDetails.first, fruDetails.second)
                    .c_str());
            return std::nullopt;
        }
        return *fruHwInvPath;
    }
}

std::optional<sdbusplus::message::object_path>
    IsolatableHWs::getClkParentFruObjPath(struct pdbg_target* clkTgt)
{
    auto clkTgtDevTreePath{pdbg_target_path(clkTgt)};

    constexpr auto MotherboardIface =
        "xyz.openbmc_project.Inventory.Item.Board.Motherboard";
    auto parentFruPath = utils::getChildsInventoryPath(
        _bus, std::string("/xyz/openbmc_project/inventory"), MotherboardIface);

    if (!parentFruPath.has_value())
    {
        log<level::ERR>(
            std::format("Failed to get the parent fru [{}] inventory path "
                        "for the given device path [{}]",
                        MotherboardIface, clkTgtDevTreePath)
                .c_str());
        return std::nullopt;
    }
    else if (parentFruPath->empty())
    {
        log<level::ERR>(
            std::format("The parent fru [{}] inventory object is not exist "
                        "for the given device path [{}]",
                        MotherboardIface, clkTgtDevTreePath)
                .c_str());
        return std::nullopt;
    }
    else if (parentFruPath->size() > 1)
    {
        log<level::ERR>(
            std::format("More than one parent fru [{}] inventory object is "
                        "exists for the given device path [{}]",
                        MotherboardIface, clkTgtDevTreePath)
                .c_str());
        return std::nullopt;
    }

    return (*parentFruPath)[0];
}

std::optional<sdbusplus::message::object_path>
    IsolatableHWs::getParentFruObjPath(struct pdbg_target* childTgt)
{
    if (childTgt == nullptr)
    {
        log<level::ERR>(
            "Given pdbg target is invalid, failed to get parent fru path");
        return std::nullopt;
    }

    auto childTgtDevTreePath{pdbg_target_path(childTgt)};

    auto pdbgTgtClass{pdbg_target_class_name(childTgt)};
    if (pdbgTgtClass == nullptr)
    {
        log<level::ERR>(
            std::format("The given hardware [{}] pdbg target class is missing, "
                        "please make sure hardware unit is added in the pdbg",
                        childTgtDevTreePath)
                .c_str());
        return std::nullopt;
    }

    /**
     * Temporary workaround to get the parent fru path for the oscrefclk
     * because the oscrefclk parent fru is not modelled in the phal
     * cec device tree.
     */
    if (strcmp(pdbgTgtClass, "oscrefclk") == 0)
    {
        return getClkParentFruObjPath(childTgt);
    }

    auto parentFruTgt = getParentFruPhalDevTreeTgt(childTgt);
    if (!parentFruTgt.has_value())
    {
        return std::nullopt;
    }

    std::string parentFruTgtPdbgClass{pdbg_target_class_name(*parentFruTgt)};
    auto parentFruHwId = IsolatableHWs::HW_Details::HwId{
        IsolatableHWs::HW_Details::HwId::PhalPdbgClassName(
            parentFruTgtPdbgClass)};

    auto parentFruHwDetails = getIsotableHWDetails(parentFruHwId);
    if (!parentFruHwDetails.has_value())
    {
        log<level::ERR>(
            std::format("Isolated hardware [{}] parent fru pdbg class [{}] is "
                        "not found in the isolatable hardware list",
                        childTgtDevTreePath, parentFruTgtPdbgClass)
                .c_str());
        return std::nullopt;
    }

    auto parentFruHwInfo = devtree::getFRUDetails(*parentFruTgt);

    auto parentFruPath = getFRUInventoryPath(
        parentFruHwInfo, parentFruHwDetails->second._invPathFuncLookUp);
    if (!parentFruPath.has_value())
    {
        log<level::ERR>(
            std::format("Failed to get get parent fru inventory path "
                        "for given device path [{}]",
                        childTgtDevTreePath)
                .c_str());
        return std::nullopt;
    }

    return parentFruPath;
}

std::optional<sdbusplus::message::object_path> IsolatableHWs::getInventoryPath(
    const devtree::DevTreePhysPath& physicalPath, bool& persistedCoreEcoMode)
{
    try
    {
        auto isolatedHwTgt = devtree::getPhalDevTreeTgt(physicalPath);
        if (!isolatedHwTgt.has_value())
        {
            return std::nullopt;
        }
        auto isolatedHwTgtDevTreePath = pdbg_target_path(*isolatedHwTgt);

        auto pdbgTgtClass{pdbg_target_class_name(*isolatedHwTgt)};
        if (pdbgTgtClass == nullptr)
        {
            log<level::ERR>(
                std::format("The given hardware [{}] pdbg target class "
                            "is missing, please make sure hardware unit "
                            "is added in pdbg ",
                            isolatedHwTgtDevTreePath)
                    .c_str());
            return std::nullopt;
        }
        std::string isolatedHwPdbgClass{pdbgTgtClass};

        std::optional<std::pair<IsolatableHWs::HW_Details::HwId,
                                IsolatableHWs::HW_Details>>
            isolatedHwDetails;
        auto isolatedHwId = IsolatableHWs::HW_Details::HwId{
            IsolatableHWs::HW_Details::HwId::PhalPdbgClassName(
                isolatedHwPdbgClass)};

        /**
         * Inventory path is different for ECO core and that need to
         * get by using the PrettyName since we need to show different
         * name for the isolated ECO mode core while listing records.
         */
        if ((isolatedHwId._pdbgClassName._name == "core") ||
            (isolatedHwId._pdbgClassName._name == "fc"))
        {
            bool ecoCore{false};

            if (isolatedHwId._pdbgClassName._name == "core")
            {
                ecoCore = devtree::isECOcore(*isolatedHwTgt);
            }
            else
            {
                struct pdbg_target* coreTgt;
                pdbg_for_each_target("core", *isolatedHwTgt, coreTgt)
                {
                    ecoCore = devtree::isECOcore(coreTgt);
                    if (ecoCore)
                    {
                        // If one of the small core is in the eco mode then,
                        // whole pair will be treated as ECO core
                        break;
                    }
                }
            }

            if (ecoCore || persistedCoreEcoMode)
            {
                isolatedHwDetails =
                    getIsolatableHWDetailsByPrettyName("Cache-Only Core");

                if (!persistedCoreEcoMode)
                {
                    persistedCoreEcoMode = true;
                }
            }
            else
            {
                isolatedHwDetails = getIsotableHWDetails(isolatedHwId);
            }
        }
        else
        {
            isolatedHwDetails = getIsotableHWDetails(isolatedHwId);
        }

        if (!isolatedHwDetails.has_value())
        {
            log<level::ERR>(
                std::format("Isolated hardware [{}] pdbg class [{}] is "
                            "not found in isolatable hardware list",
                            isolatedHwTgtDevTreePath, isolatedHwPdbgClass)
                    .c_str());
            return std::nullopt;
        }

        sdbusplus::message::object_path isolatedHwInventoryPath;
        if (isolatedHwDetails->second._isItFRU)
        {
            auto isolatedHwInfo = devtree::getFRUDetails(*isolatedHwTgt);

            auto inventoryPath = getFRUInventoryPath(
                isolatedHwInfo, isolatedHwDetails->second._invPathFuncLookUp);
            if (!inventoryPath.has_value())
            {
                log<level::ERR>(std::format("Failed to get inventory path for "
                                            "given device path [{}]",
                                            isolatedHwTgtDevTreePath)
                                    .c_str());
                return std::nullopt;
            }

            isolatedHwInventoryPath = *inventoryPath;
        }
        else
        {
            auto parentFruPath = getParentFruObjPath(*isolatedHwTgt);
            if (!parentFruPath.has_value())
            {
                return std::nullopt;
            }

            auto childsInventoryPath = utils::getChildsInventoryPath(
                _bus, *parentFruPath,
                isolatedHwDetails->first._interfaceName._name);
            if (!childsInventoryPath.has_value())
            {
                return std::nullopt;
            }

            /**
             * If the isolated hardware inventory item interface is
             * CommonInventoryItemIface ("xyz.openbmc_project.Inventory.Item")
             * then, use PrettyName as unique hardware id because currently
             * few isolatbale hardware subunits is not modelled in the BMC
             * Inventory and Redfish so those subunits need to look based on
             * the PrettyName to get inventory path.
             */
            inv_path_lookup_func::UniqueHwId uniqIsolateHwKey;
            if (isolatedHwDetails->first._interfaceName._name ==
                CommonInventoryItemIface)
            {
                uniqIsolateHwKey = isolatedHwDetails->second._prettyName;
            }
            else
            {
                // TODO Below decision need to be based on system core mode
                //     i.e whether need to use "fc" (in big core system) or
                //     "core" (in small core system) pdbg target class to get
                //     the appropriate target physical path from the phal
                //     cec device tree but, now using the "fc".
                if (isolatedHwPdbgClass == "core")
                {
                    struct pdbg_target* parentFc =
                        pdbg_target_parent("fc", *isolatedHwTgt);
                    if (parentFc == nullptr)
                    {
                        log<level::ERR>(
                            std::format("Failed to get the parent FC "
                                        "target for the given device tree "
                                        "target path [{}]",
                                        isolatedHwTgtDevTreePath)
                                .c_str());
                        return std::nullopt;
                    }
                    uniqIsolateHwKey =
                        devtree::getHwInstIdFromDevTree(parentFc);
                }
                else
                {
                    uniqIsolateHwKey =
                        devtree::getHwInstIdFromDevTree(*isolatedHwTgt);
                }
            }

            auto isolateHwPath = std::find_if(
                childsInventoryPath->begin(), childsInventoryPath->end(),
                [&uniqIsolateHwKey, &isolatedHwDetails,
                 this](const auto& path) {
                return isolatedHwDetails->second._invPathFuncLookUp(
                    this->_bus, path, uniqIsolateHwKey);
                });

            if (isolateHwPath == childsInventoryPath->end())
            {
                log<level::ERR>(std::format("Failed to get inventory path for "
                                            "given device path [{}]",
                                            isolatedHwTgtDevTreePath)
                                    .c_str());
                return std::nullopt;
            }

            isolatedHwInventoryPath = *isolateHwPath;
        }
        return isolatedHwInventoryPath;
    }
    catch (const std::exception& e)
    {
        log<level::ERR>(std::format("Exception [{}]", e.what()).c_str());
        return std::nullopt;
    }
}

} // namespace isolatable_hws

namespace inv_path_lookup_func
{

IsItIsoHwInvPath itemInstanceId(sdbusplus::bus::bus& /* bus */,
                                const sdbusplus::message::object_path& objPath,
                                const UniqueHwId& instanceId)
{
    if (!std::holds_alternative<type::InstanceId>(instanceId))
    {
        // Should not happen, could be wrong lookup function
        return false;
    }

    auto objInstId = utils::getInstanceId(objPath.filename());
    if (!objInstId.has_value())
    {
        return false;
    }

    return *objInstId == std::get<type::InstanceId>(instanceId);
}

IsItIsoHwInvPath itemPrettyName(sdbusplus::bus::bus& bus,
                                const sdbusplus::message::object_path& objPath,
                                const UniqueHwId& prettyName)
{
    if (!std::holds_alternative<std::string>(prettyName))
    {
        // Should not happen, could be wrong lookup function
        return false;
    }

    try
    {
        auto retPrettyName = utils::getDBusPropertyVal<std::string>(
            bus, objPath, "xyz.openbmc_project.Inventory.Item", "PrettyName");
	if(std::get<std::string>(prettyName)=="OpenCAPI Memory Buffer")
		return retPrettyName == "OpenCAPI Memory Buffer 2A";

        return retPrettyName == std::get<std::string>(prettyName);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::WARNING>(std::format("Exception [{}] to get PrettyName for "
                                        "the given object path [{}]",
                                        e.what(), objPath.str)
                                .c_str());
        return false;
    }
}

IsItIsoHwInvPath
    itemLocationCode(sdbusplus::bus::bus& bus,
                     const sdbusplus::message::object_path& objPath,
                     const UniqueHwId& locCode)
{
    if (!std::holds_alternative<type::LocationCode>(locCode))
    {
        // Should not happen, could be wrong lookup function
        return false;
    }

    try
    {
        auto expandedLocCode = utils::getDBusPropertyVal<std::string>(
            bus, objPath,
            "xyz.openbmc_project.Inventory.Decorator.LocationCode",
            "LocationCode");

        auto unExpandedLocCode{devtree::getUnexpandedLocCode(expandedLocCode)};
        if (!unExpandedLocCode.has_value())
        {
            return false;
        }

        return *unExpandedLocCode == std::get<type::LocationCode>(locCode);
    }
    catch (const sdbusplus::exception::exception& e)
    {
        log<level::WARNING>(std::format("Exception [{}] to get LocationCode "
                                        "for the given object path [{}]",
                                        e.what(), objPath.str)
                                .c_str());
        return false;
    }
}

} // namespace inv_path_lookup_func

} // namespace hw_isolation
