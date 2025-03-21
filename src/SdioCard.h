/**
 * Copyright (c) 2011-2021 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef SdioCard_h
#define SdioCard_h
#include <stddef.h>
#include "imxrt.h"
#include "SdCardInterface.h"
#include "ioctl.h"

#define FIFO_SDIO 0
#define DMA_SDIO 1

#define USE_SDIO2 2

/**
 * SD maximum initialization clock rate.
 */
#ifndef SD_MAX_INIT_RATE_KHZ
#define SD_MAX_INIT_RATE_KHZ 400
#endif  // SD_MAX_INIT_RATE_KHZ
/**/

#if 0 //defined(__IMXRT1062__)
// Temporary experiment 
// Define as class -> move to imxrt.h
typedef struct {
        volatile uint32_t DS_ADDR;                  //offset000)
        volatile uint32_t BLK_ATT;                  //offset004)
        volatile uint32_t CMD_ARG;                  //offset008)
        volatile uint32_t CMD_XFR_TYP;              //offset00C)
        volatile uint32_t CMD_RSP0;                 //offset010)
        volatile uint32_t CMD_RSP1;                 //offset014)
        volatile uint32_t CMD_RSP2;                 //offset018)
        volatile uint32_t CMD_RSP3;                 //offset01C)
        volatile uint32_t DATA_BUFF_ACC_PORT;       //offset020)
        volatile uint32_t PRES_STATE;               //offset024)
        volatile uint32_t PROT_CTRL;                //offset028)
        volatile uint32_t SYS_CTRL;                 //offset02C)
        volatile uint32_t INT_STATUS;               //offset030)
        volatile uint32_t INT_STATUS_EN;            //offset034)
        volatile uint32_t INT_SIGNAL_EN;            //offset038)
        volatile uint32_t AUTOCMD12_ERR_STATUS;     //offset03C)
        volatile uint32_t HOST_CTRL_CAP;            //offset040)
        volatile uint32_t WTMK_LVL;                 //offset044)
        volatile uint32_t MIX_CTRL;                 //offset048)
        uint32_t unused1;
        volatile uint32_t FORCE_EVENT;              //offset050)
        volatile uint32_t ADMA_ERR_STATUS;          //offset054)
        volatile uint32_t ADMA_SYS_ADDR;            //offset058)
        uint32_t unused2;
        volatile uint32_t DLL_CTRL;                 //offset060)
        volatile uint32_t DLL_STATUS;               //offset064)
        volatile uint32_t CLK_TUNE_CTRL_STATUS;     //offset068)
        uint32_t unused3[21];                       //6c 70 80 90 A0 B0
        volatile uint32_t VEND_SPEC;                //offset0C0)
        volatile uint32_t MMC_BOOT;                 //offset0C4)
        volatile uint32_t VEND_SPEC2;               //offset0C8)
        volatile uint32_t TUNING_CTRL;              //offset0CC)
} IMXRT_USDHC_t;

#endif


//------------------------------------------------------------------------------
/**
 * \class W4343WCard
 * \brief Raw SDIO access to SD and SDHC flash memory cards.
 */
class W4343WCard  {
 public:
  /** Initialize the SD card.
   * \param[in] useSDIO2 SDIO card configuration.
   * \return true for success or false for failure.
   */
  bool begin(bool useSDIO2, int8_t wlOnPin, int8_t wlIrqPin, int8_t extLPOPin = -1);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    uint32_t __attribute__((error("use sectorCount()"))) cardSize();
#endif  // DOXYGEN_SHOULD_SKIP_THIS
    /**
     * \return code for the last error. See SdCardInfo.h for a list of error codes.
     */
    uint8_t errorCode() const;
    /** \return error data for last error. */
    uint32_t errorData() const;
    /** \return error line for last error. Tmp function for debug. */
    uint32_t errorLine() const;
    /** \return the SD clock frequency in kHz. */
    uint32_t kHzSdClk();

    ///////////////////////////
    // IRW new public functions
    ///////////////////////////
    void getMACAddress();
    void getFirmwareVersion();
    void getCLMVersion();
    void postInitSettings();
    void pollEvents();
    void ScanNetworks();
    void JoinNetworks(const char *ssID, const char *passphrase, int security);
  ///////////////////////////////
  // End IRW new public functions
  ///////////////////////////////
  
  private:
    static const uint8_t IDLE_STATE = 0;
    static const uint8_t READ_STATE = 1;
    static const uint8_t WRITE_STATE = 2;
    uint32_t m_curSector;
    uint8_t m_curState = IDLE_STATE;
    static volatile bool fUseSDIO2;
    int8_t m_wlIrqPin = -1;

    // move helper functions into class.
    typedef bool (W4343WCard::*pcheckfcn)();
    bool cardCommand(uint32_t xfertyp, uint32_t arg);
    void enableSDIO(bool enable);
    void enableDmaIrs();
    void initSDHC();
    bool isBusyCommandComplete();
    bool isBusyCommandInhibit();
    void setSdclk(uint32_t kHzMax);
    bool waitTimeout(pcheckfcn fcn);
    inline bool setSdErrorCode(uint8_t code, uint32_t line);
    
    /////////////////////////////////////////////
    void printRegs(uint32_t line);
    static void sdISR();
    static void sdISR2(); // one for second SDIO
    static W4343WCard *s_pSdioCards[2];
    void gpioMux(uint8_t mode);
    void initClock();
    uint32_t baseClock();
    
    ////////////////////
    // IRW new functions
    ////////////////////
  void makeSDIO_DAT1();
  void makeGPIO_DAT1();
  bool SDIOEnableFunction(uint8_t functionEnable);
  bool SDIODisableFunction(uint8_t functionEnable);
  bool configureOOBInterrupt();
  static volatile bool dataISRReceived;
  static void onWLIRQInterruptHandler();
  static void onDataInterruptHandler();
  bool prepareDataTransfer(uint8_t *buffer, uint16_t length) ;
  bool completeDataTransfer() ;
  
  bool cardCMD52_read(uint32_t functionNumber, uint32_t registerAddress, uint8_t * buffer, bool logOutput = true);
  bool cardCMD52_write(uint32_t functionNumber, uint32_t registerAddress, uint8_t data, bool logOutput = true);
  bool cardCMD52(uint32_t functionNumber, uint32_t registerAddress, uint8_t data, bool write, bool readAfterWriteFlag, uint8_t * response, bool logOutput);
  
  void setBlockCountSize(bool blockMode, uint32_t functionNumber, uint32_t size);
  bool cardCMD53_read(uint32_t functionNumber, uint32_t registerAddress, uint8_t * buffer, uint32_t size, bool logOutput = true);
  bool cardCMD53_write(uint32_t functionNumber, uint32_t registerAddress, uint8_t * buffer, uint32_t size, bool logOutput = true);
  bool cardCMD53(uint32_t functionNumber, uint32_t registerAddress, uint8_t * buffer, uint32_t size, bool write, bool logOutput);
  
  void setBackplaneWindow(uint32_t addr);
  uint32_t setBackplaneWindow_retOffset(uint32_t addr);
  uint32_t backplaneWindow_read32(uint32_t addr, uint32_t *valp);
  uint32_t backplaneWindow_write32(uint32_t addr, uint32_t val);

  bool checkValidFirmware(size_t len, uintptr_t source);
  bool uploadFirmware(size_t firmwareSize, uintptr_t source);
  bool uploadNVRAM(size_t nvRAMSize, uintptr_t source);
  bool uploadCLM();

  int set_iovar_mpc(uint8_t val);
  int ioctl_get_uint32(const char * name, int wait_msec,  uint8_t *data);
  int ioctl_set_uint32(const char *name, int wait_msec, uint32_t val);
  int ioctl_set_intx2(const char *name, int wait_msec, int32_t val1, int32_t val2);
  int ioctl_wr_int32(int cmd, int wait_msec, int val);
  int ioctl_get_data(const char *name, int wait_msec, uint8_t *data, int dlen, bool logOutput = true);
  int ioctl_set_data(const char *name, int wait_msec, void *data, int len, bool logOutput = true);
  int ioctl_wr_data(int cmd, int wait_msec, void *data, int len);
  int ioctl_rd_data(int cmd, int wait_msec, void *data, int len);
  int ioctl_cmd(int cmd, const char *name, int wait_msec, int wr, void *data, int dlen, bool logOutput = true);
  int ioctl_cmd_poll_device(int wait_msec, int wr, void *data, int dlen, bool logOutput = true);
  int ioctl_wait(int usec);
  uint32_t ioctl_get_event(sdpcm_header_t *hp, uint8_t *data, int maxlen);
  int ioctl_enable_evts(EVT_STR *evtp);  
  const char *ioctl_evt_status_str(int status);
  const char *ioctl_evt_str(int event);

  void printResponse(bool return_value);
  void printMACAddress(uint8_t * data);
  void printSSID(uint8_t * data);
  void disp_fields(void *data, char *fields, int maxlen);
  void disp_block(uint8_t *data, int len);
  void disp_bytes(uint8_t *data, int len);

  ////////////////////////
  // End IRW new functions
  ////////////////////////
  
  bool isBusyDat();
  bool isBusyFifoRead();
  bool isBusyFifoWrite();
  bool isBusyTransferComplete();

  bool waitTransferComplete();

  /////////////////////////////////////////////////////
  // lets move global (static) variables into class instance.
  IMXRT_USDHC_t *m_psdhc;
  pcheckfcn m_busyFcn = nullptr;
  bool m_initDone = false;
  bool m_version2;
  bool m_highCapacity;
  bool m_transferActive = false;
  uint8_t m_errorCode = SD_CARD_ERROR_INIT_NOT_CALLED;
  uint32_t m_errorLine = 0;
  uint32_t m_rca;
  volatile bool m_dmaBusy = false;
  volatile uint32_t m_irqstat;
  uint32_t m_sdClkKhz = 0;
  uint32_t m_ocr;
  cid_t m_cid;
  csd_t m_csd;
};

#endif  // SdioCard_h
