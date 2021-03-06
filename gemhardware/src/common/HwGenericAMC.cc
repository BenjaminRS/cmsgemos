#include "gem/hw/HwGenericAMC.h"

#include <iomanip>

// // define the consts
// const unsigned gem::hw::HwGenericAMC::N_GTX;

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionFile),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_INFO("HwGenericAMC ctor");
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC");

  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionURI,
                                    std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionURI, addressTable),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_INFO("trying to create HwGenericAMC(" << amcDevice << "," << connectionURI << "," <<addressTable);
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC");
  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice,uhalDevice),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC"); // needed?
  if (isConnected) {
    this->loadModule("amc", "amc v1.0.1");
    CMSGEMOS_DEBUG("HwGenericAMC::HwGenericAMC amc module loaded");
  } else {
    CMSGEMOS_WARN("HwGenericAMC::HwGenericAMC RPC interface failed to connect");
  }

  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC::HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

// gem::hw::HwGenericAMC::HwGenericAMC(int const& shelf,
//                                     int const& slot,
//                                     bool       uhalNative) :
//   gem::hw::HwGenericAMC::HwGenericAMC(toolbox::toString("gem-shelf%02d-amc%02d",shelf,slot),
//                                       connectionURI,
//                                       addressTable
//                                     )
// {
// }

gem::hw::HwGenericAMC::~HwGenericAMC()
{
}

void gem::hw::HwGenericAMC::connectRPC(bool reconnect)
{
  if (isConnected) {
    // TODO: find better way than hardcoded versions
    this->loadModule("amc", "amc v1.0.1");
    CMSGEMOS_DEBUG("HwGenericAMC::connectRPC modules loaded");
  } else {
    CMSGEMOS_WARN("HwGenericAMC::connectRPC RPC interface failed to connect");
  }
}

bool gem::hw::HwGenericAMC::isHwConnected()
{
  // DO NOT LIKE THIS FUNCTION FIXME!!!
  if (b_is_connected) {
    CMSGEMOS_INFO("basic check: HwGenericAMC connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    CMSGEMOS_INFO("basic check: HwGenericAMC pointer valid");
    std::vector<linkStatus> tmp_activeLinks; // FIXME SHOULD BE READ FROM FW, WHAT IS THIS USED FOR?
    m_maxLinks = this->getSupportedOptoHybrids();
    tmp_activeLinks.reserve(m_maxLinks);
    if ((this->getBoardID()).rfind("GLIB") != std::string::npos ) {
      CMSGEMOS_INFO("HwGenericAMC found boardID");
      for (unsigned int gtx = 0; gtx < m_maxLinks; ++gtx) {
        // FIXME!!! somehow need to actually check that the specified link is present
        b_links[gtx] = true;
        CMSGEMOS_INFO("m_links 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << m_links
                      << " 0x1 << gtx = " << std::setw(8) << std::setfill('0') << (0x1<<gtx)
                      << std::dec);
        m_links |= (0x1<<gtx);
        CMSGEMOS_INFO("gtx" << gtx << " present(" << this->getFirmwareVer() << ")");
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                      << m_links << std::dec);
        tmp_activeLinks.push_back(std::make_pair(gtx,this->LinkStatus(gtx)));
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                      << m_links << std::dec);
      }
    } else {
      CMSGEMOS_WARN("Device not reachable (unable to find 'GLIB' in the board ID)"
                    << " board ID "              << this->getBoardID()
                    << " user firmware version " << this->getFirmwareVer());
      return false;
    }

    v_activeLinks = tmp_activeLinks;
    if (!v_activeLinks.empty()) {
      b_is_connected = true;
      CMSGEMOS_DEBUG("checked gtxs: HwGenericAMC connection good");
      return true;
    } else {
      b_is_connected = false;
      return false;
    }
  } else {
    return false;
  }
}

std::string gem::hw::HwGenericAMC::getBoardID()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  res = gem::utils::uint32ToString(getBoardIDRaw());
  return res;
}

uint32_t gem::hw::HwGenericAMC::getBoardIDRaw()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(), "GEM_SYSTEM.BOARD_ID");
  return val;
}

std::string gem::hw::HwGenericAMC::getFirmwareDate(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid = getFirmwareDateRaw(system);

  // GLIB system FW 0x1f0222b7
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)       // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f)  // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);                // year
  // GEM system FW date 0xYYYYMMDD
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareDateRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE.DATE");
  else
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE.DATE");
}

std::string gem::hw::HwGenericAMC::getFirmwareVer(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid;

  // GLIB system FW 0x1f0222b7
  fwid = getFirmwareVerRaw(system);
  res << ((fwid>>16)&0xff) << "."
      << ((fwid>>8) &0xff) << "."
      << ((fwid)    &0xff);
  // GEM system FW version 0xMMmmpppp
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareVerRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
  else
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
}

/** User core functionality **/
uint32_t gem::hw::HwGenericAMC::getUserFirmware()
{
  // This returns the firmware register (V2 removed the user firmware specific).
  return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
}

std::string gem::hw::HwGenericAMC::getUserFirmwareDate()
{
  // This returns the user firmware build date.
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware() << std::dec;
  return res.str();
}

bool gem::hw::HwGenericAMC::linkCheck(uint8_t const& gtx, std::string const& opMsg)
{
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  if (gtx > m_maxLinks) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, m_maxLinks);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
    //  } else if (!b_links[gtx]) {
  } else if (!((m_links>>gtx)&0x1)) {
    CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                  << m_links << std::dec);
    std::string msg = toolbox::toString("%s requested inactive gtx (%d, 0x%08x, 0x%08x)",
                                        opMsg.c_str(), gtx,m_links,m_links>>gtx);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
  }
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::HwGenericAMC::LinkStatus(uint8_t const& gtx)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;
  // FIXME moved to OH GEM_AMC.OH.OH0.FPGA.GBT.RX.CNT_LINK_ERR
  CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  if (linkCheck(gtx, "Link status")) {
    linkStatus.GTX_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GTX_TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("TRIGGER.OH%d.LINK0_MISSED_COMMA_CNT",gtx));
    linkStatus.GTX_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
    linkStatus.GBT_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GBT_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
  }
  CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  return linkStatus;
}

void gem::hw::HwGenericAMC::LinkReset(uint8_t const& gtx, uint8_t const& resets)
{
  // FIXME changed in V3 firmware
  // req = wisc::RPCMsg("amc.linkReset");
  // req.set_word("NOH",NOH);
  // try {
  //   rsp = rpc.call_method(req);
  // } STANDARD_CATCH;

  // try {
  //   if (rsp.get_key_exists("error")) {
  //     ERROR("LinkReset error: " << rsp.get_string("error").c_str());
  //     //throw xhal::utils::XHALException("DAQ_TRIGGER_MAIN update failed");
  //   }
  // } STANDARD_CATCH;

  // right now this just resets the counters, but we need to be able to "reset" the link too
  if (linkCheck(gtx, "Link reset")) {
    if (resets&0x1)
      writeReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
    if (resets&0x2)
      writeReg(getDeviceBaseNode(),toolbox::toString("TRIGGER.CTRL.CNT_RESET"),  gtx);
    if (resets&0x4)
      writeReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
  }
}


gem::hw::HwGenericAMC::AMCIPBusCounters gem::hw::HwGenericAMC::getIPBusCounters(uint8_t const& gtx,
                                                                                uint8_t const& mode)
{
  // FIXME removed in V3 firmware
  if (linkCheck(gtx, "IPBus counter")) {
    if (mode&0x01)
      m_ipBusCounters.at(gtx).OptoHybridStrobe = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x02)
      m_ipBusCounters.at(gtx).OptoHybridAck    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x04)
      m_ipBusCounters.at(gtx).TrackingStrobe   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x08)
      m_ipBusCounters.at(gtx).TrackingAck      = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x10)
      m_ipBusCounters.at(gtx).CounterStrobe    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters"));
    if (mode&0x20)
      m_ipBusCounters.at(gtx).CounterAck       = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters"));
  }
  return m_ipBusCounters.at(gtx);
}

void gem::hw::HwGenericAMC::resetIPBusCounters(uint8_t const& gtx, uint8_t const& resets)
{
  // FIXME removed in V3 firmware
  if (linkCheck(gtx, "Reset IPBus counters")) {
    if (resets&0x01)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d.Reset",gtx), 0x1);
    if (resets&0x02)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d.Reset",   gtx), 0x1);
    if (resets&0x04)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.TRK_%d.Reset",       gtx), 0x1);
    if (resets&0x08)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.TRK_%d.Reset",          gtx), 0x1);
    if (resets&0x10)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters.Reset"),          0x1);
    if (resets&0x20)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters.Reset"),             0x1);
  }
}

uint32_t gem::hw::HwGenericAMC::readTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return 0;
}

void gem::hw::HwGenericAMC::flushTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return;
}

/** DAQ link module functions **/
void gem::hw::HwGenericAMC::configureDAQModule(bool enableZS, bool doPhaseShift, uint32_t const& runType, uint32_t const& marker, bool relock, bool bc0LockPSMode)
{
  try {
    req = wisc::RPCMsg("amc.configureDAQModule");
    req.set_word("enableZS",     enableZS);
    req.set_word("doPhaseShift", doPhaseShift);
    req.set_word("runType",      runType);
    req.set_word("marker",       marker);
    req.set_word("relock",       relock);
    req.set_word("bc0LockPSMode",bc0LockPSMode);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::configureDAQModule: " << errmsg.str());
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::exception::Exception);
}

void gem::hw::HwGenericAMC::enableDAQLink(uint32_t const& enableMask)
{
  try {
    req = wisc::RPCMsg("amc.enableDAQLink");
    req.set_word("enableMask",enableMask);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::enableDAQLink: " << errmsg.str());
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::exception::Exception);
}

void gem::hw::HwGenericAMC::disableDAQLink()
{
  try {
    req = wisc::RPCMsg("amc.disableDAQLink");
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::disableDAQLink: " << errmsg.str());
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::exception::Exception);
}

void gem::hw::HwGenericAMC::setZS(bool en)
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.ZERO_SUPPRESSION_EN", uint32_t(en));
}

void gem::hw::HwGenericAMC::resetDAQLink(uint32_t const& davTO, uint32_t const& ttsOverride)
{
  try {
    req = wisc::RPCMsg("amc.resetDAQLink");
    req.set_word("davTO",       davTO);
    req.set_word("ttsOverride", ttsOverride);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
    /*  FIXME alternative
    // should reraise gem::hw::exception::RPCMethodError as a result of the STANDARD_CATCH
    // turn this into a macro?
    } catch (...) {
      handleRPCError("HwGenericAMC::enableDAQLink");
      // https://stackoverflow.com/questions/3561659/how-can-i-abstract-out-a-repeating-try-catch-pattern-in-c
      // https://stackoverflow.com/questions/2466131/is-re-throwing-an-exception-legal-in-a-nested-try
      // https://stackoverflow.com/questions/13007793/is-a-macro-to-catch-a-set-of-exceptions-at-different-places-fine
      // with lambdas
      // https://codereview.stackexchange.com/questions/2484/generic-c-exception-catch-handler-macro
    }
     */
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::exception::Exception);

  /**
     need to wrap any exceptions into gem::hw::exception exceptions, or handle them as STANDARD_CATCH simply throws:
     xhal::utils::XHALRPCNotConnectedException
     xhal::utils::XHALRPCException
   */
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkControl()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkStatus()
{
  // FIXME: can't read non-terminal register by node name in rwreg
  return readReg(getDeviceBaseNode(), "DAQ.STATUS");
}

bool gem::hw::HwGenericAMC::daqLinkReady()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_LINK_RDY");
}

bool gem::hw::HwGenericAMC::daqClockLocked()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_CLK_LOCKED");
}

bool gem::hw::HwGenericAMC::daqTTCReady()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.TTC_RDY");
}

uint8_t gem::hw::HwGenericAMC::daqTTSState()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.TTS_STATE");
}

bool gem::hw::HwGenericAMC::daqAlmostFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_AFULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsEmpty()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_EMPTY");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsAlmostFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_NEAR_FULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_FULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsUnderflow()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_UNDERFLOW");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkEventsSent()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.EVT_SENT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkL1AID()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.L1AID");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDisperErrors()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.DISPER_ERR");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkNonidentifiableErrors()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.NOTINTABLE_ERR");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputMask()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDAVTimeout()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL.DAV_TIMEOUT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDAVTimer(bool const& max)
{
  if (max)
    return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.MAX_DAV_TIMER");
  else
    return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.LAST_DAV_TIMER");
}

/** GTX specific DAQ link information **/
// TODO: should rename, "DAQ link" is a back end FW term, not corresponding to a front end link...
uint32_t gem::hw::HwGenericAMC::getLinkDAQStatus(uint8_t const& gtx)
{
  // do link protections here...
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".STATUS");
}

uint32_t gem::hw::HwGenericAMC::getLinkDAQCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg(getDeviceBaseNode(),regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg(getDeviceBaseNode(),regBase.str()+".EVN");
}

uint32_t gem::hw::HwGenericAMC::getLinkLastDAQBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputTimeout()
{
  // OBSOLETE, NO LONGER PRESENT
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.INPUT_TIMEOUT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunType()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_TYPE");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunParameters()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_PARAMS");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunParameter(uint8_t const& parameter)
{
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  return readReg(getDeviceBaseNode(),regBase.str());
}

void gem::hw::HwGenericAMC::setDAQLinkInputTimeout(uint32_t const& value)
{
  // for (unsigned li = 0; li < m_maxLinks; ++li) {
  // for (unsigned li =  m_maxLinks - 1; li > -1; --li) {
  //   writeReg(getDeviceBaseNode(), toolbox::toString("DAQ.OH%d.CONTROL.EOE_TIMEOUT", li), value);
  // }
  // return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.INPUT_TIMEOUT",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunType(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_TYPE",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunParameters(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_PARAMS",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value)
{
  if (parameter < 1 || parameter > 3) {
    std::string msg = toolbox::toString("Attempting to set DAQ link run parameter %d: outside expectation (1-%d)",
                                        (int)parameter,3);
    CMSGEMOS_ERROR(msg);
    return;
  }
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  writeReg(getDeviceBaseNode(),regBase.str(),value);
}

/********************************/
/** TTC module information **/
/********************************/

/** TTC module functions **/
void gem::hw::HwGenericAMC::ttcModuleReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::HwGenericAMC::ttcMMCMReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.MMCM_RESET", 0x1);
  // writeReg(getDeviceBaseNode(), "TTC.CTRL.PHASE_ALIGNMENT_RESET", 0x1);
}

void gem::hw::HwGenericAMC::ttcMMCMPhaseShift(bool relock, bool modeBC0, bool scan)
{
  try {
    req = wisc::RPCMsg("amc.ttcMMCMPhaseShift");
    req.set_word("relock",  relock);
    req.set_word("modeBC0", modeBC0);
    req.set_word("scan",    scan);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::ttcMMCMPhaseShift", gem::hw::exception::Exception);
}

int gem::hw::HwGenericAMC::checkPLLLock(uint32_t readAttempts)
{
  try {
    req = wisc::RPCMsg("amc.checkPLLLock");
    req.set_word("readAttempts", readAttempts);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
      return rsp.get_word("lockCnt");
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::checkPLLLock", gem::hw::exception::Exception);
}

double gem::hw::HwGenericAMC::getMMCMPhaseMean(uint32_t readAttempts)
{
  if (readAttempts == 1) {
    return double(readReg(getDeviceBaseNode(), "TTC.STATUS.CLK.TTC_PM_PHASE_MEAN"));
  } else {
    try {
      req = wisc::RPCMsg("amc.getMMCMPhaseMean");
      req.set_word("reads", readAttempts);
      try {
        rsp = rpc.call_method(req);
        try {
          if (rsp.get_key_exists("error")) {
            std::stringstream errmsg;
            errmsg << rsp.get_string("error");
            XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
          }
        } STANDARD_CATCH;
        return rsp.get_word("phase");
      } STANDARD_CATCH;
    } GEM_CATCH_RPC_ERROR("HwGenericAMC::getMMCMPhaseMean", gem::hw::exception::Exception);
  }
}

double gem::hw::HwGenericAMC::getMMCMPhaseMedian(uint32_t readAttempts)
{
  CMSGEMOS_WARN("HwGenericAMC::getMMCMPhaseMedian is not implemented, returning the mean");
  return getMMCMPhaseMean(readAttempts);
}

double gem::hw::HwGenericAMC::getGTHPhaseMean(uint32_t readAttempts)
{
  if (readAttempts == 1) {
    return readReg(getDeviceBaseNode(), "TTC.STATUS.CLK.GTH_PM_PHASE_MEAN");
  } else {
    try {
      req = wisc::RPCMsg("amc.getGTHPhaseMean");
      req.set_word("reads", readAttempts);
      try {
        rsp = rpc.call_method(req);
        try {
          if (rsp.get_key_exists("error")) {
            std::stringstream errmsg;
            errmsg << rsp.get_string("error");
            XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
          }
        } STANDARD_CATCH;
        return rsp.get_word("phase");
      } STANDARD_CATCH;
    } GEM_CATCH_RPC_ERROR("HwGenericAMC::getGTHPhaseMean", gem::hw::exception::Exception);
  }
}

double gem::hw::HwGenericAMC::getGTHPhaseMedian(uint32_t readAttempts)
{
  CMSGEMOS_WARN("HwGenericAMC::getGTHPhaseMedian is not implemented, returning the mean");
  return getGTHPhaseMean(readAttempts);
}

void gem::hw::HwGenericAMC::ttcCounterReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.CNT_RESET", 0x1);
}

bool gem::hw::HwGenericAMC::getL1AEnable()
{
  return readReg(getDeviceBaseNode(), "TTC.CTRL.L1A_ENABLE");
}

void gem::hw::HwGenericAMC::setL1AEnable(bool enable)
{
  // uint32_t safeEnable = 0xa4a2c200+int(enable);
  writeReg(getDeviceBaseNode(), "TTC.CTRL.L1A_ENABLE", uint32_t(enable));
}

uint32_t gem::hw::HwGenericAMC::getTTCConfig(AMCTTCCommandT const& cmd)
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCConfig: not implemented");
  return 0x0;
}

void gem::hw::HwGenericAMC::setTTCConfig(AMCTTCCommandT const& cmd, uint8_t const& value)
{
  CMSGEMOS_WARN("HwGenericAMC::setTTCConfig: not implemented");
  return;
}

uint32_t gem::hw::HwGenericAMC::getTTCStatus()
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCStatus: not fully implemented");
  return readReg(getDeviceBaseNode(), "TTC.STATUS");
}

uint32_t gem::hw::HwGenericAMC::getTTCErrorCount(bool const& single)
{
  if (single)
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_SINGLE_ERROR_CNT");
  else
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_DOUBLE_ERROR_CNT");
}

uint32_t gem::hw::HwGenericAMC::getTTCCounter(AMCTTCCommandT const& cmd)
{
  switch(cmd) {
  case(AMCTTCCommand::TTC_L1A) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.L1A");
  case(AMCTTCCommand::TTC_BC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.BC0");
  case(AMCTTCCommand::TTC_EC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.EC0");
  case(AMCTTCCommand::TTC_RESYNC) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.RESYNC");
  case(AMCTTCCommand::TTC_OC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.OC0");
  case(AMCTTCCommand::TTC_HARD_RESET) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.HARD_RESET");
  case(AMCTTCCommand::TTC_CALPULSE) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.CALPULSE");
  case(AMCTTCCommand::TTC_START) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.START");
  case(AMCTTCCommand::TTC_STOP) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.STOP");
  case(AMCTTCCommand::TTC_TEST_SYNC) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.TEST_SYNC");
  default :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.L1A");
  }
}

uint32_t gem::hw::HwGenericAMC::getL1AID()
{
  return readReg(getDeviceBaseNode(), "TTC.L1A_ID");
}

uint32_t gem::hw::HwGenericAMC::getL1ARate()
{
  return readReg(getDeviceBaseNode(), "TTC.L1A_RATE");
}

uint32_t gem::hw::HwGenericAMC::getTTCSpyBuffer()
{
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::getTTCSpyBuffer: TTC.TTC_SPY_BUFFER is obsolete and will be removed in a future release");
  return 0x0;
  // return readReg(getDeviceBaseNode(), "TTC.TTC_SPY_BUFFER");
}

/********************************/
/** SLOW_CONTROL module information **/
/********************************/

/*** SCA submodule ***/
void gem::hw::HwGenericAMC::scaHardResetEnable(bool const& en)
{
  writeReg(getDeviceBaseNode(), "SLOW_CONTROL.SCA.CTRL.TTC_HARD_RESET_EN", uint32_t(en));
}

/********************************/
/** TRIGGER module information **/
/********************************/

void gem::hw::HwGenericAMC::triggerReset()
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::HwGenericAMC::triggerCounterReset()
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.CNT_RESET", 0x1);
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridKillMask()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.CTRL.OH_KILL_MASK");
}

void gem::hw::HwGenericAMC::setOptoHybridKillMask(uint32_t const& mask)
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.OH_KILL_MASK", mask);
}

/*** STATUS submodule ***/
uint32_t gem::hw::HwGenericAMC::getORTriggerRate()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.STATUS.OR_TRIGGER_RATE");
}

uint32_t gem::hw::HwGenericAMC::getORTriggerCount()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.STATUS.TRIGGER_SINGLE_ERROR_CNT");
}

/*** OH{IDXX} submodule ***/
uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerRate(uint8_t const& oh)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.TRIGGER_RATE",(int)oh));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerCount(uint8_t const& oh)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.TRIGGER_CNT",(int)oh));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridClusterRate(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_RATE",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridClusterCount(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_CNT",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridDebugLastCluster(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.DEBUG_LAST_CLUSTER_%d",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerLinkCount(uint8_t const& oh, uint8_t const& link, AMCOHLinkCountT const& count)
{
  switch(count) {
  case(AMCOHLinkCount::LINK_NOT_VALID) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_NOT_VALID_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_MISSED_COMMA) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_OVERFLOW) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_OVERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_UNDERFLOW) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_UNDERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_SYNC_WORD) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_SYNC_WORD_CNT",(int)oh,(int)link));
  default :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  }
}


// general resets
void gem::hw::HwGenericAMC::generalReset()
{
  // TODO: CTP7 module candidate
  // reset all counters
  counterReset();

  for (unsigned gtx = 0; gtx < m_maxLinks; ++gtx)
    linkReset(gtx);

  // other resets

  return;
}

void gem::hw::HwGenericAMC::counterReset()
{
  // TODO: CTP7 module candidate
  // reset all counters
  resetT1Counters();

  for (unsigned gtx = 0; gtx < m_maxLinks; ++gtx)
    resetIPBusCounters(gtx, 0xff);

  linkCounterReset();

  return;
}

void gem::hw::HwGenericAMC::resetT1Counters()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3
  CMSGEMOS_WARN("HwGenericAMC::resetT1Counters is obsolete and will be removed in a future release");
  writeReg(getDeviceBaseNode(), "T1.L1A.RESET",      0x1);
  writeReg(getDeviceBaseNode(), "T1.CalPulse.RESET", 0x1);
  writeReg(getDeviceBaseNode(), "T1.Resync.RESET",   0x1);
  writeReg(getDeviceBaseNode(), "T1.BC0.RESET",      0x1);
  return;
}

void gem::hw::HwGenericAMC::linkCounterReset()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::linkCounterReset: not yet implemented");
  return;
}

void gem::hw::HwGenericAMC::linkReset(uint8_t const& gtx)
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::linkReset: not yet implemented");
  linkCounterReset();
  return;
}
