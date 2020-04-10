/**
 * @brief SI4735 ARDUINO LIBRARY  
 * 
 * @details This is an Arduino library for the SI47XX, BROADCAST AM/FM/SW RADIO RECEIVER IC family from Silicon Labs. 
 * @details This library is intended to provide an easier interface for controlling the SI47XX by using Arduino platform. 
 * @details The communication used by this library is I2C.
 * @details This file contains: const (#define), Defined Data type and Methods declarations
 * @details You can see a complete documentation on <https://github.com/pu2clr/SI4735>
 * @details The are more than 20 examples on <https://github.com/pu2clr/SI4735/tree/master/examples>
 *   
 * @see https://pu2clr.github.io/SI4735/
 * @see Si47XX PROGRAMMING GUIDE AN332: https://www.silabs.com/documents/public/application-notes/AN332.pdf
 * @see AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; AMENDMENT FOR SI4735-D60 SSB AND NBFM PATCHES
 *  
 * @author PU2CLR - Ricardo Lima Caratti 
 * @date  2019-2020
 */

#include <Arduino.h>
#include <Wire.h>

#define POWER_UP_FM 0  // FM
#define POWER_UP_AM 1  // AM and SSB (if patch applyed)
#define POWER_UP_WB 3  // Weather Band Receiver
#define POWER_PATCH 15 //

// SI473X commands (general)
#define SI473X_ADDR_SEN_LOW 0x11  // SI473X I2C buss address when the SEN pin (16) is set to low 0V.
#define SI473X_ADDR_SEN_HIGH 0x63 // SI473X I2C buss address when the SEN pin (16) is set to high +3.3V

#define POWER_UP 0x01       // Power up device and mode selection.
#define GET_REV 0x10        // Returns revision information on the device.
#define POWER_DOWN 0x11     // Power down device.
#define SET_PROPERTY 0x12   // Sets the value of a property.
#define GET_PROPERTY 0x13   // Retrieves a property’s value.
#define GET_INT_STATUS 0x14 // Read interrupt status bits.

// FM
#define FM_TUNE_FREQ 0x20
#define FM_SEEK_START 0x21 // Begins searching for a valid FM frequency.
#define FM_TUNE_STATUS 0x22
#define FM_AGC_STATUS 0x27
#define FM_AGC_OVERRIDE 0x28
#define FM_RSQ_STATUS 0x23
#define FM_RDS_STATUS 0x24 // Returns RDS information for current channel and reads an entry from the RDS FIFO.

// FM RDS properties
#define FM_RDS_INT_SOURCE 0x1500
#define FM_RDS_INT_FIFO_COUNT 0x1501
#define FM_RDS_CONFIG 0x1502
#define FM_RDS_CONFIDENCE 0x1503

#define FM_BLEND_STEREO_THRESHOLD 0x1105
#define FM_BLEND_MONO_THRESHOLD 0x1106
#define FM_BLEND_RSSI_STEREO_THRESHOLD 0x1800
#define FM_BLEND_RSSI_MONO_THRESHOLD 0x1801
#define FM_BLEND_SNR_STEREO_THRESHOLD 0x1804
#define FM_BLEND_SNR_MONO_THRESHOLD 0x1805
#define FM_BLEND_MULTIPATH_STEREO_THRESHOLD 0x1808
#define FM_BLEND_MULTIPATH_MONO_THRESHOLD 0x1809

// AM command
#define AM_TUNE_FREQ 0x40    // Tunes to a given AM frequency.
#define AM_SEEK_START 0x41   // Begins searching for a valid AM frequency.
#define AM_TUNE_STATUS 0x42  // Queries the status of the already issued AM_TUNE_FREQ or AM_SEEK_START command.
#define AM_RSQ_STATUS 0x43   // Queries the status of the Received Signal Quality (RSQ) for the current channel.
#define AM_AGC_STATUS 0x47   // Queries the current AGC settings.
#define AM_AGC_OVERRIDE 0x48 // Overrides AGC settings by disabling and forcing it to a fixed value.
#define GPIO_CTL 0x80        // Configures GPO1, 2, and 3 as output or Hi-Z.
#define GPIO_SET 0x81        // Sets GPO1, 2, and 3 output level (low or high).

//SSB command (SAME AM CMD VALUES)
// See AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; pages 4 and 5
#define SSB_TUNE_FREQ 0x40    // Tunes to a given SSB frequency.
#define SSB_TUNE_STATUS 0x42  // Queries the status of the already issued SSB_TUNE_FREQ or AM_SEEK_START command.
#define SSB_RSQ_STATUS 0x43   // Queries the status of the Received Signal Quality (RSQ) for the current channel.
#define SSB_AGC_STATUS 0x47   // Queries the current AGC settings.
#define SSB_AGC_OVERRIDE 0x48 // Overrides AGC settings by disabling and forcing it to a fixed value.

// AM/SW/LW Receiver Property Summary
// See  Si47XX PROGRAMMING GUIDE AN332; page 125
#define DIGITAL_OUTPUT_FORMAT 0x0102                // Configure digital audio outputs.
#define DIGITAL_OUTPUT_SAMPLE_RATE 0x0104           // Configure digital audio output sample rate
#define REFCLK_FREQ 0x0201                          //Sets frequency of reference clock in Hz. The range is 31130 to 34406 Hz, or 0 to disable the AFC. Default is 32768 Hz.
#define REFCLK_PRESCALE 0x0202                      // Sets the prescaler value for RCLK input.
#define AM_DEEMPHASIS 0x3100                        // Sets deemphasis time constant. Can be set to 50 μs. Deemphasis is disabled by default.
#define AM_CHANNEL_FILTER 0x3102                    // Selects the bandwidth of the channel filter for AM reception. The choices are 6, 4, 3, 2, 2.5, 1.8, or 1 (kHz). The default bandwidth is 2 kHz.
#define AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN 0x3103 // Sets the maximum gain for automatic volume control.
#define AM_MODE_AFC_SW_PULL_IN_RANGE 0x3104         // Sets the SW AFC pull-in range.
#define AM_MODE_AFC_SW_LOCK_IN_RANGE 0x3105         // Sets the SW AFC lock-in.
#define AM_RSQ_INTERRUPTS 0x3200                    // Same SSB - Configures interrupt related to Received Signal Quality metrics. All interrupts are disabled by default.
#define AM_RSQ_SNR_HIGH_THRESHOLD 0x3201            //Sets high threshold for SNR interrupt.
#define AM_RSQ_SNR_LOW_THRESHOLD 0x3202             // Sets low threshold for SNR interrupt.
#define AM_RSQ_RSSI_HIGH_THRESHOLD 0x3203           // Sets high threshold for RSSI interrupt.
#define AM_RSQ_RSSI_LOW_THRESHOLD 0x3204            // Sets low threshold for RSSI interrupt.
#define AM_SOFT_MUTE_RATE 0x3300                    // Sets the attack and decay rates when entering or leaving soft mute. The default is 278 dB/s.
#define AM_SOFT_MUTE_SLOPE 0x3301                   // Sets the AM soft mute slope. Default value is a slope of 1.
#define AM_SOFT_MUTE_MAX_ATTENUATION 0x3302         // Sets maximum attenuation during soft mute (dB). Set to 0 to disable soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_SNR_THRESHOLD 0x3303           // Sets SNR threshold to engage soft mute. Default is 8 dB.
#define AM_SOFT_MUTE_RELEASE_RATE 0x3304            // Sets softmute release rate. Smaller values provide slower release, and larger values provide faster release.
#define AM_SOFT_MUTE_ATTACK_RATE 0x3305             // Sets software attack rate. Smaller values provide slower attack, and larger values provide faster attack.
#define AM_SEEK_BAND_BOTTOM 0x3400                  // Sets the bottom of the AM band for seek. Default is 520.
#define AM_SEEK_BAND_TOP 0x3401                     // Sets the top of the AM band for seek. Default is 1710.
#define AM_SEEK_FREQ_SPACING 0x3402                 // Selects frequency spacing for AM seek. Default is 10 kHz spacing.
#define AM_SEEK_SNR_THRESHOLD 0x3403                // Sets the SNR threshold for a valid AM Seek/Tune.
#define AM_SEEK_RSSI_THRESHOLD 0x3404               // Sets the RSSI threshold for a valid AM Seek/Tune.
#define AM_AGC_ATTACK_RATE 0x3702                   // Sets the number of milliseconds the high peak detector must be exceeded before decreasing gain.
#define AM_AGC_RELEASE_RATE 0x3703                  // Sets the number of milliseconds the low peak detector must not be exceeded before increasing the gain.
#define AM_FRONTEND_AGC_CONTROL 0x3705              // Adjusts AM AGC for frontend (external) attenuator and LNA.
#define AM_NB_DETECT_THRESHOLD 0x3900               // Sets the threshold for detecting impulses in dB above the noise floor
#define AM_NB_INTERVAL 0x3901                       // Interval in micro-seconds that original samples are replaced by interpolated clean samples
#define AM_NB_RATE 0x3902                           // Noise blanking rate in 100 Hz units. Default value is 64.
#define AM_NB_IIR_FILTER 0x3903                     // Sets the bandwidth of the noise floor estimator. Default value is 300.
#define AM_NB_DELAY 0x3904                          // Delay in micro-seconds before applying impulse blanking to the original samples

#define RX_VOLUME 0x4000
#define RX_HARD_MUTE 0x4001

// SSB properties
// See AN332 REV 0.8 Universal Programming Guide (Amendment for SI4735-D60 SSN and NBFM Patches)

#define GPO_IEN 0x0001                       // AM and SSB - Enable interrupt source
#define SSB_BFO 0x0100                       // Sets the Beat Frequency Offset (BFO) under SSB mode.
#define SSB_MODE 0x0101                      // Sets number of properties of the SSB mode.
#define SSB_RSQ_INTERRUPTS 0x3200            // Configure Interrupts related to RSQ
#define SSB_RSQ_SNR_HI_THRESHOLD 0x3201      // Sets high threshold for SNR interrupt
#define SSB_RSQ_SNR_LO_THRESHOLD 0x3202      // Sets low threshold for SNR interrupt
#define SSB_RSQ_RSSI_HI_THRESHOLD 0x3203     // Sets high threshold for RSSI interrupt
#define SSB_RSQ_RSSI_LO_THRESHOLD 0x3204     // Sets low threshold for RSSI interrupt
#define SSB_SOFT_MUTE_RATE 0x3300            // Sets the attack and decay rates when entering or leaving soft mute
#define SSB_SOFT_MUTE_MAX_ATTENUATION 0x3302 // Sets the maximum attenuation during soft mute (db); 0dB to disable soft mute; defaul 8dB;
#define SSB_SOFT_MUTE_SNR_THRESHOLD 0x3303   // Sets SNR threshould to engage soft mute. Defaul 8dB
#define SSB_RF_AGC_ATTACK_RATE 0x3700        // Sets the number of milliseconds the high RF peak detector must be exceeded before decreasing the gain. Defaul 4.
#define SSB_RF_AGC_RELEASE_RATE 0x3701       // Sets the number of milliseconds the low RF peak detector must be exceeded before increasing the gain. Defaul 24.

// SSB
#define SSB_RF_IF_AGC_ATTACK_RATE 0x3702  // Sets the number of milliseconds the high IF peak detector must be exceeded before decreasing gain. Defaul 4.
#define SSB_RF_IF_AGC_RELEASE_RATE 0x3703 // Sets the number of milliseconds the low IF peak detector must be exceeded before increasing the gain. Defaul 140.

// See AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; pages 12 and 13
#define LSB_MODE 1 // 01
#define USB_MODE 2 // 10

// Parameters
#define SI473X_ANALOG_AUDIO 0b00000101   // Analog Audio Inputs
#define SI473X_DIGITAL_AUDIO1 0b00001011 // Digital audio output (DCLK, LOUT/DFS, ROUT/DIO)
#define SI473X_DIGITAL_AUDIO2 0b10110000 // Digital audio outputs (DCLK, DFS, DIO)
#define SI473X_DIGITAL_AUDIO3 0b10110101 // Analog and digital audio outputs (LOUT/ROUT and DCLK, DFS,DIO)

// Other parameters
#define FM_CURRENT_MODE 0
#define AM_CURRENT_MODE 1
#define SSB_CURRENT_MODE 2

#define MAX_DELAY_AFTER_SET_FREQUENCY 30 // In ms - This value helps to improve the precision during of getting frequency value
#define MAX_DELAY_AFTER_POWERUP 10       // In ms - Max delay you have to setup after a power up command.
#define MIN_DELAY_WAIT_SEND_LOOP 300     // In uS (Microsecond) - each loop of waitToSend sould wait this value in microsecond

/** @defgroup group01 SI473X data types 
 * @section group01 SI473X data types 
 *  
 * @brief SI473X data representation
 * 
 * @details The goal here is separate data from code. 
 * The Si47XX family works with many internal data that can be represented by data structure 
 * or defined data type in C/C++. These C/C++ resources have been used widely here.  
 * This aproach made the library easier to build and maintain.  Each data structure created 
 * here has its reference (name of the document and page on which it was based). 
 * In other words, to make the SI47XX device easier to deal, some defined data types were 
 * created to handle byte and bits to process  commands, properties and responses.
 * These data types will be usefull to deal with SI473X 
 */

/**
 * @ingroup group01
 * 
 * @brief Power Up arguments data type 
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 64 and 65
 */
typedef union {
    struct
    {
        // ARG1
        uint8_t FUNC : 4;    //!<  Function (0 = FM Receive; 1–14 = Reserved; 15 = Query Library ID)
        uint8_t XOSCEN : 1;  //!<  Crystal Oscillator Enable (0 = crystal oscillator disabled; 1 = Use crystal oscillator and and OPMODE=ANALOG AUDIO) .
        uint8_t PATCH : 1;   //!<  Patch Enable (0 = Boot normally; 1 = Copy non-volatile memory to RAM).
        uint8_t GPO2OEN : 1; //!<  GPO2 Output Enable (0 = GPO2 output disabled; 1 = GPO2 output enabled).
        uint8_t CTSIEN : 1;  //!<  CTS Interrupt Enable (0 = CTS interrupt disabled; 1 = CTS interrupt enabled).
        // ARG2
        uint8_t OPMODE; //!<  Application Setting. See page 65
    } arg;              //!<  Refined powerup parameters
    uint8_t raw[2];     //!<  Raw powerup parameters data. Same arg memory position. So, same content.
} si473x_powerup;

/**
 * @ingroup group01
 * 
 * @brief Represents how the  frequency is stored in the si4735.
 * @details It helps to convert frequency in uint16_t to two bytes (uint8_t) (FREQL and FREQH)  
 */
typedef union {
    struct
    {
        uint8_t FREQL; //!<  Tune Frequency Low byte.
        uint8_t FREQH; //!<  Tune Frequency High byte.
    } raw;             //!<  Raw data that represents the frequency stored in the Si47XX device.
    uint16_t value;    //!<  frequency (integer value)
} si47x_frequency;

/**
 * @ingroup group01
 * @brief Antenna Tuning Capacitor data type manupulation 
 */
typedef union {
    struct
    {
        uint8_t ANTCAPL; //!<  Antenna Tuning Capacitor High byte
        uint8_t ANTCAPH; //!<  Antenna Tuning Capacitor Low byte
    } raw;
    uint16_t value;
} si47x_antenna_capacitor;

/**
 * @ingroup group01
 * 
 * @brief AM Tune frequency data type command (AM_TUNE_FREQ command)
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 135
 */
typedef union {
    struct
    {
        uint8_t FAST : 1;   //!<  ARG1 - FAST Tuning. If set, executes fast and invalidated tune. The tune status will not be accurate.
        uint8_t FREEZE : 1; //!<  Valid only for FM (Must be 0 to AM)
        uint8_t DUMMY1 : 4; //!<  Always set 0
        uint8_t USBLSB : 2; //!<  SSB Upper Side Band (USB) and Lower Side Band (LSB) Selection. 10 = USB is selected; 01 = LSB is selected.
        uint8_t FREQH;      //!<  ARG2 - Tune Frequency High byte.
        uint8_t FREQL;      //!<  ARG3 - Tune Frequency Low byte.
        uint8_t ANTCAPH;    //!<  ARG4 - Antenna Tuning Capacitor High byte.
        uint8_t ANTCAPL;    //!<  ARG5 - Antenna Tuning Capacitor Low byte. Note used for FM.
    } arg;
    uint8_t raw[5];
} si47x_set_frequency;

/** 
 * @ingroup group01 
 * 
 * @brief Seek frequency (automatic tuning)
 * 
 * @details Represents searching for a valid frequency data type.
 */
typedef union {
    struct
    {
        uint8_t RESERVED1 : 2;
        uint8_t WRAP : 1;   //!<  Determines whether the seek should Wrap = 1, or Halt = 0 when it hits the band limit.
        uint8_t SEEKUP : 1; //!<  Determines the direction of the search, either UP = 1, or DOWN = 0.
        uint8_t RESERVED2 : 4;
    } arg;
    uint8_t raw;
} si47x_seek;

/** 
 * @ingroup group01 status response structure
 * 
 * @brief Status response data representation
 * 
 * @details Represents searching for a valid frequency data type.
 */
typedef union {
    struct
    {
        uint8_t STCINT : 1; //!< 1 = Tune complete has been triggered.
        uint8_t DUMMY1 : 1; //!< Reserved (Values may vary).
        uint8_t RDSINT : 1; //!< 1 = Radio data system interrupt has been triggered.
        uint8_t RSQINT : 1; //!< 1 = Received Signal Quality measurement has been triggered.
        uint8_t DUMMY2 : 2; //!< Reserved (Values may vary).
        uint8_t ERR : 1; //!< 1 = Error.
        uint8_t CTS : 1; //!< 0 = Wait before sending next command; 1 = Clear to send next command.
    } refined;
    uint8_t raw;
} si47x_status;

/**  
 * @ingroup group01
 * 
 * @brief Response status command 
 * 
 * @details Response data from a query status command
 * 
 * @see Si47XX PROGRAMMING GUIDE; pages 73 and 
 */
typedef union {
    struct
    {
        // Status
        uint8_t STCINT : 1; //!<  Seek/Tune Complete Interrupt; 1 = Tune complete has been triggered.
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1; //!<  Radio Data System (RDS) Interrup; 0 = interrupt has not been triggered.
        uint8_t RSQINT : 1; //!<  Received Signal Quality Interrupt; 0 = interrupt has not been triggered.
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1; //!<  Error. 0 = No error 1 = Error
        uint8_t CTS : 1; //!<  Clear to Send.
        // RESP1
        uint8_t VALID : 1; //!<  Valid Channel
        uint8_t AFCRL : 1; //!<  AFC Rail Indicator
        uint8_t DUMMY3 : 5;
        uint8_t BLTF : 1; //!<  Reports if a seek hit the band limit
        // RESP2
        uint8_t READFREQH; //!<  Read Frequency High byte.
        // RESP3
        uint8_t READFREQL; //!<  Read Frequency Low byte.
        // RESP4
        uint8_t RSSI; //!<  Received Signal Strength Indicator (dBμV)
        // RESP5
        uint8_t SNR; //!<  This byte contains the SNR metric when tune is complete (dB).
        // RESP6
        uint8_t MULT; //!<  Contains the multipath metric when tune is complete
        // RESP7
        uint8_t READANTCAP; //!<  Contains the current antenna tuning capacitor value
    } resp;
    uint8_t raw[8]; //!<  Check it
} si47x_response_status;

/**
 * @ingroup group01
 * 
 * @brief Data representation for  Firmware Information (GET_REV)
 * 
 * @details The part number, chip revision, firmware revision, patch revision and component revision numbers. 
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 66 and 131 
 */
typedef union {
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1;
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        uint8_t PN;       //!<  RESP1 - Final 2 digits of Part Number (HEX).
        uint8_t FWMAJOR;  //!<  RESP2 - Firmware Major Revision (ASCII).
        uint8_t FWMINOR;  //!<  RESP3 - Firmware Minor Revision (ASCII).
        uint8_t PATCHH;   //!<  RESP4 - Patch ID High byte (HEX).
        uint8_t PATCHL;   //!<  RESP5 - Patch ID Low byte (HEX).
        uint8_t CMPMAJOR; //!<  RESP6 - Component Major Revision (ASCII).
        uint8_t CMPMINOR; //!<  RESP7 - Component Minor Revision (ASCII).
        uint8_t CHIPREV;  //!<  RESP8 - Chip Revision (ASCII).
        // RESP9 to RESP15 not used
    } resp;
    uint8_t raw[9];
} si47x_firmware_information;

/**
 * @ingroup group01
 * 
 * @brief Firmware Query Library ID response. 
 * 
 * @details Used to represent the response of a power up command with FUNC = 15 (patch)
 * 
 * To confirm that the patch is compatible with the internal device library revision, the library 
 * revision should be confirmed by issuing the POWER_UP command with Function = 15 (query library ID)
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; page 12 
 */
typedef union {
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1;
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        uint8_t PN;        //!<  RESP1 - Final 2 digits of Part Number (HEX).
        uint8_t FWMAJOR;   //!<  RESP2 - Firmware Major Revision (ASCII).
        uint8_t FWMINOR;   //!<  RESP3 - Firmware Minor Revision (ASCII).
        uint8_t RESERVED1; //!<  RESP4 - Reserved, various values.
        uint8_t RESERVED2; //!<  RESP5 - Reserved, various values.
        uint8_t CHIPREV;   //!<  RESP6 - Chip Revision (ASCII).
        uint8_t LIBRARYID; //!<  RESP7 - Library Revision (HEX).
        // RESP9 to RESP15 not used
    } resp;
    uint8_t raw[8];
} si47x_firmware_query_library;

/**
 * @ingroup group01
 * 
 * @brief Seek station status
 * 
 * @details Status of FM_TUNE_FREQ or FM_SEEK_START commands or Status of AM_TUNE_FREQ or AM_SEEK_START commands.
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 73 and 139
 */
typedef union {
    struct
    {
        uint8_t INTACK : 1; //!<  If set, clears the seek/tune complete interrupt status indicator.
        uint8_t CANCEL : 1; //!<  If set, aborts a seek currently in progress.
        uint8_t RESERVED2 : 6;
    } arg;
    uint8_t raw;
} si47x_tune_status;

/**
 * @ingroup group01
 * 
 * @brief Data type to deal with SET_PROPERTY command
 * 
 * @details Property Data type (help to deal with SET_PROPERTY command on si473X)
 */
typedef union {
    struct
    {
        uint8_t byteLow;
        uint8_t byteHigh;
    } raw;
    uint16_t value;
} si47x_property;

/** @defgroup group02 RDS Data types */
/** @section group02 RDS Data types */

/** 
 * @ingroup group02
 * 
 * @brief  Radio Signal Quality data representation 
 * 
 * @details Data type for status information about the received signal quality (FM_RSQ_STATUS and AM_RSQ_STATUS)
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 75 and 
 */
typedef union {
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1;
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        // RESP1
        uint8_t RSSIILINT : 1; //!<  RSSI Detect Low.
        uint8_t RSSIHINT : 1;  //!<  RSSI Detect High.
        uint8_t SNRLINT : 1;   //!<  SNR Detect Low.
        uint8_t SNRHINT : 1;   //!<  SNR Detect High.
        uint8_t MULTLINT : 1;  //!<  Multipath Detect Low
        uint8_t MULTHINT : 1;  //!<  Multipath Detect High
        uint8_t DUMMY3 : 1;
        uint8_t BLENDINT : 1; //!<  Blend Detect Interrupt.
        // RESP2
        uint8_t VALID : 1; //!<  Valid Channel.
        uint8_t AFCRL : 1; //!<  AFC Rail Indicator.
        uint8_t DUMMY4 : 1;
        uint8_t SMUTE : 1; //!<  Soft Mute Indicator. Indicates soft mute is engaged.
        uint8_t DUMMY5 : 4;
        // RESP3
        uint8_t STBLEND : 7; //!<  Indicates amount of stereo blend in% (100 = full stereo, 0 = full mono).
        uint8_t PILOT : 1;   //!<  Indicates stereo pilot presence.
        // RESP4 to RESP7
        uint8_t RSSI;    //!<  RESP4 - Contains the current receive signal strength (0–127 dBμV).
        uint8_t SNR;     //!<  RESP5 - Contains the current SNR metric (0–127 dB).
        uint8_t MULT;    //!<  RESP6 - Contains the current multipath metric. (0 = no multipath; 100 = full multipath)
        uint8_t FREQOFF; //!<  RESP7 - Signed frequency offset (kHz).
    } resp;
    uint8_t raw[8];
} si47x_rqs_status;

/**
 * @ingroup group02
 * 
 * @brief Data type for RDS Status command and response information 
 *
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 77 and 78 
 * @see Also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union {
    struct
    {
        uint8_t INTACK : 1;     // Interrupt Acknowledge; 0 = RDSINT status preserved; 1 = Clears RDSINT.
        uint8_t MTFIFO : 1;     // Empty FIFO; 0 = If FIFO not empty; 1 = Clear RDS Receive FIFO.
        uint8_t STATUSONLY : 1; // Determines if data should be removed from the RDS FIFO.
        uint8_t dummy : 5;
    } arg;
    uint8_t raw;
} si47x_rds_command;

/**
 * @ingroup group02
 * 
 * @brief Response data type for current channel and reads an entry from the RDS FIFO.
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 77 and 78
 */
typedef union {
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1;
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        // RESP1
        uint8_t RDSRECV : 1;      //!<  RDS Received; 1 = FIFO filled to minimum number of groups set by RDSFIFOCNT.
        uint8_t RDSSYNCLOST : 1;  //!<  RDS Sync Lost; 1 = Lost RDS synchronization.
        uint8_t RDSSYNCFOUND : 1; //!<  RDS Sync Found; 1 = Found RDS synchronization.
        uint8_t DUMMY3 : 1;
        uint8_t RDSNEWBLOCKA : 1; //!<  RDS New Block A; 1 = Valid Block A data has been received.
        uint8_t RDSNEWBLOCKB : 1; //!<  RDS New Block B; 1 = Valid Block B data has been received.
        uint8_t DUMMY4 : 2;
        // RESP2
        uint8_t RDSSYNC : 1; //!<  RDS Sync; 1 = RDS currently synchronized.
        uint8_t DUMMY5 : 1;
        uint8_t GRPLOST : 1; //!<  Group Lost; 1 = One or more RDS groups discarded due to FIFO overrun.
        uint8_t DUMMY6 : 5;
        // RESP3 to RESP11
        uint8_t RDSFIFOUSED; //!<  RESP3 - RDS FIFO Used; Number of groups remaining in the RDS FIFO (0 if empty).
        uint8_t BLOCKAH;     //!<  RESP4 - RDS Block A; HIGH byte
        uint8_t BLOCKAL;     //!<  RESP5 - RDS Block A; LOW byte
        uint8_t BLOCKBH;     //!<  RESP6 - RDS Block B; HIGH byte
        uint8_t BLOCKBL;     //!<  RESP7 - RDS Block B; LOW byte
        uint8_t BLOCKCH;     //!<  RESP8 - RDS Block C; HIGH byte
        uint8_t BLOCKCL;     //!<  RESP9 - RDS Block C; LOW byte
        uint8_t BLOCKDH;     //!<  RESP10 - RDS Block D; HIGH byte
        uint8_t BLOCKDL;     //!<  RESP11 - RDS Block D; LOW byte
        // RESP12 - Blocks A to D Corrected Errors.
        // 0 = No errors;
        // 1 = 1–2 bit errors detected and corrected;
        // 2 = 3–5 bit errors detected and corrected.
        // 3 = Uncorrectable.
        uint8_t BLED : 2;
        uint8_t BLEC : 2;
        uint8_t BLEB : 2;
        uint8_t BLEA : 2;
    } resp;
    uint8_t raw[13];
} si47x_rds_status;

/**
 * @ingroup group02
 * 
 * @brief FM_RDS_INT_SOURCE property data type
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; page 103
 * @see also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union {
    struct
    {
        uint8_t RDSRECV : 1;      //!<  If set, generate RDSINT when RDS FIFO has at least FM_RDS_INT_FIFO_COUNT entries.
        uint8_t RDSSYNCLOST : 1;  //!<  If set, generate RDSINT when RDS loses synchronization.
        uint8_t RDSSYNCFOUND : 1; //!<  f set, generate RDSINT when RDS gains synchronization.
        uint8_t DUMMY1 : 1;       //!<  Always write to 0.
        uint8_t RDSNEWBLOCKA : 1; //!<  If set, generate an interrupt when Block A data is found or subsequently changed
        uint8_t RDSNEWBLOCKB : 1; //!<  If set, generate an interrupt when Block B data is found or subsequently changed
        uint8_t DUMMY2 : 5;       //!<  Reserved - Always write to 0.
        uint8_t DUMMY3 : 5;       //!<  Reserved - Always write to 0.
    } refined;
    uint8_t raw[2];
} si47x_rds_int_source;

/**
 * @ingroup group02
 * 
 * @brief Data type for FM_RDS_CONFIG Property
 * 
 * IMPORTANT: all block errors must be less than or equal the associated block error threshold for the group 
 * to be stored in the RDS FIFO. 
 * 0 = No errors; 1 = 1–2 bit errors detected and corrected; 2 = 3–5 bit errors detected and corrected; 3 = Uncorrectable.
 * Recommended Block Error Threshold options:
 *  2,2,2,2 = No group stored if any errors are uncorrected.
 *  3,3,3,3 = Group stored regardless of errors.
 *  0,0,0,0 = No group stored containing corrected or uncorrected errors.
 *  3,2,3,3 = Group stored with corrected errors on B, regardless of errors on A, C, or D.
 *  
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 58 and 104 
 */
typedef union {
    struct
    {
        uint8_t RDSEN : 1; //!<  1 = RDS Processing Enable.
        uint8_t DUMMY1 : 7;
        uint8_t BLETHD : 2; //!<  Block Error Threshold BLOCKD
        uint8_t BLETHC : 2; //!<  Block Error Threshold BLOCKC.
        uint8_t BLETHB : 2; //!<  Block Error Threshold BLOCKB.
        uint8_t BLETHA : 2; //!<  Block Error Threshold BLOCKA.
    } arg;
    uint8_t raw[2];
} si47x_rds_config;

/**
 * @ingroup group02
 * 
 * @brief Block A data type
 */
typedef union {
    struct
    {
        uint16_t pi;
    } refined;
    struct
    {
        uint8_t highValue; // Most Significant uint8_t first
        uint8_t lowValue;
    } raw;
} si47x_rds_blocka;

/**
 * @ingroup group02
 * 
 * @brief Block B data type
 * 
 * @details For GCC on System-V ABI on 386-compatible (32-bit processors), the following stands:
 * 
 * 1) Bit-fields are allocated from right to left (least to most significant).
 * 2) A bit-field must entirely reside in a storage unit appropriate for its declared type.
 *    Thus a bit-field never crosses its unit boundary.
 * 3) Bit-fields may share a storage unit with other struct/union members, including members that are not bit-fields.
 *    Of course, struct members occupy different parts of the storage unit.
 * 4) Unnamed bit-fields' types do not affect the alignment of a structure or union, although individual 
 *    bit-fields' member offsets obey the alignment constraints.   
 * 
 * @see also Si47XX PROGRAMMING GUIDE; AN332; pages 78 and 79
 * @see also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union {
    struct
    {
        uint16_t address : 2;            // Depends on Group Type and Version codes. If 0A or 0B it is the Text Segment Address.
        uint16_t DI : 1;                 // Decoder Controll bit
        uint16_t MS : 1;                 // Music/Speech
        uint16_t TA : 1;                 // Traffic Announcement
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group0;
    struct
    {
        uint16_t address : 4;            // Depends on Group Type and Version codes. If 2A or 2B it is the Text Segment Address.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } group2;
    struct
    {
        uint16_t content : 4;            // Depends on Group Type and Version codes.
        uint16_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint16_t programType : 5;        // PTY (Program Type) code
        uint16_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint16_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint16_t groupType : 4;          // Group Type code.
    } refined;
    struct
    {
        uint8_t lowValue;
        uint8_t highValue; // Most Significant byte first
    } raw;
} si47x_rds_blockb;

/*
 * 
 * 
 * Group type 4A ( RDS Date and Time)
 * When group type 4A is used by the station, it shall be transmitted every minute according to EN 50067.
 * This Structure uses blocks 2,3 and 5 (B,C,D)
 * 
 * Commented due to “Crosses boundary” on GCC 32-bit plataform.
 */
/*
typedef union {
    struct
    {
        uint32_t offset : 5;       // Local Time Offset
        uint32_t offset_sense : 1; // Local Offset Sign ( 0 = + , 1 = - )
        uint32_t minute : 6;       // UTC Minutes
        uint32_t hour : 5;         // UTC Hours
        uint32_t mjd : 17;        // Modified Julian Day Code
    } refined;
    uint8_t raw[6];
} si47x_rds_date_time;
*/

/**
 * @ingroup group02
 * 
 * Group type 4A ( RDS Date and Time)
 * When group type 4A is used by the station, it shall be transmitted every minute according to EN 50067.
 * This Structure uses blocks 2,3 and 5 (B,C,D)
 * 
 * ATTENTION: 
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * split minute and hour representation. 
 */
typedef union {
    struct
    {
        uint8_t offset : 5;       // Local Time Offset
        uint8_t offset_sense : 1; // Local Offset Sign ( 0 = + , 1 = - )
        uint8_t minute1 : 2;      // UTC Minutes - 2 bits less significant (void “Crosses boundary”).
        uint8_t minute2 : 4;      // UTC Minutes - 4 bits  more significant  (void “Crosses boundary”)
        uint8_t hour1 : 4;        // UTC Hours - 4 bits less significant (void “Crosses boundary”)
        uint8_t hour2 : 1;        // UTC Hours - 4 bits more significant (void “Crosses boundary”)
        uint32_t mjd : 17;        // Modified Julian Day Code
    } refined;
    uint8_t raw[6];
} si47x_rds_date_time;

/** @defgroup group03 Receiver Status and Setup */
/** @section group03 Receiver Status and Setup */

/** 
 * @ingroup group03
 * 
 * AGC data types
 * FM / AM and SSB structure to AGC
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; For FM page 80; for AM page 142
 * @see AN332 REV 0.8 Universal Programming Guide Amendment for SI4735-D60 SSB and NBFM patches; page 18. 
 */
typedef union {
    struct
    {
        // status ("RESP0")
        uint8_t STCINT : 1;
        uint8_t DUMMY1 : 1;
        uint8_t RDSINT : 1; // Not used for AM/SSB
        uint8_t RSQINT : 1;
        uint8_t DUMMY2 : 2;
        uint8_t ERR : 1;
        uint8_t CTS : 1;
        // RESP1
        uint8_t AGCDIS : 1; // This bit indicates if the AGC is enabled or disabled. 0 = AGC enabled; 1 = AGC disabled.
        uint8_t DUMMY : 7;
        // RESP2
        uint8_t AGCIDX; // For FM (5 bits - READ_LNA_GAIN_INDEX - 0 = Minimum attenuation (max gain)). For AM (8 bits). This byte reports the current AGC gain index.
    } refined;
    uint8_t raw[3];
} si47x_agc_status;

/** 
 * @ingroup group03
 * 
 * If FM, Overrides AGC setting by disabling the AGC and forcing the LNA to have a certain gain that ranges between 0 
 * (minimum attenuation) and 26 (maximum attenuation).
 * If AM, overrides the AGC setting by disabling the AGC and forcing the gain index that ranges between 0
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; For FM page 81; for AM page 143
 */
typedef union {
    struct
    {
        // ARG1
        uint8_t AGCDIS : 1; // if set to 1 indicates if the AGC is disabled. 0 = AGC enabled; 1 = AGC disabled.
        uint8_t DUMMY : 7;
        // ARG2
        uint8_t AGCIDX; // AGC Index; If AMAGCDIS = 1, this byte forces the AGC gain index; 0 = Minimum attenuation (max gain)
    } arg;
    uint8_t raw[2];
} si47x_agc_overrride;

/** 
 * @ingroup group03
 * 
 * The bandwidth of the AM channel filter data type
 * AMCHFLT values: 0 = 6 kHz Bandwidth                    
 *                 1 = 4 kHz Bandwidth
 *                 2 = 3 kHz Bandwidth
 *                 3 = 2 kHz Bandwidth
 *                 4 = 1 kHz Bandwidth
 *                 5 = 1.8 kHz Bandwidth
 *                 6 = 2.5 kHz Bandwidth, gradual roll off
 *                 7–15 = Reserved (Do not use)
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; pages 125 and 151
 */
typedef union {
    struct
    {
        uint8_t AMCHFLT : 4; //!<  Selects the bandwidth of the AM channel filter.
        uint8_t DUMMY1 : 4;
        uint8_t AMPLFLT : 1; //!<  Enables the AM Power Line Noise Rejection Filter.
        uint8_t DUMMY2 : 7;
    } param;
    uint8_t raw[2];
} si47x_bandwidth_config; // AM_CHANNEL_FILTER

/** 
 * @ingroup group03
 * 
 * SSB - datatype for SSB_MODE (property 0x0101)
 * 
 * @see AN332 REV 0.8 UNIVERSAL PROGRAMMING GUIDE; page 24 
 */
typedef union {
    struct
    {
        uint8_t AUDIOBW : 4;     //!<  0 = 1.2KHz (default); 1=2.2KHz; 2=3KHz; 3=4KHz; 4=500Hz; 5=1KHz
        uint8_t SBCUTFLT : 4;    //!<  SSB side band cutoff filter for band passand low pass filter
        uint8_t AVC_DIVIDER : 4; //!<  set 0 for SSB mode; set 3 for SYNC mode;
        uint8_t AVCEN : 1;       //!<  SSB Automatic Volume Control (AVC) enable; 0=disable; 1=enable (default);
        uint8_t SMUTESEL : 1;    //!<  SSB Soft-mute Based on RSSI or SNR
        uint8_t DUMMY1 : 1;      //!<  Always write 0;
        uint8_t DSP_AFCDIS : 1;  //!<  0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
    } param;
    uint8_t raw[2];
} si47x_ssb_mode;

/**
 * @ingroup group03
 * 
 * @brief Digital audio output format data structure (Property 0x0102. DIGITAL_OUTPUT_FORMAT).
 * 
 * @details Used to configure: DCLK edge, data format, force mono, and sample precision.
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; page 195. 
 */
typedef union {
    struct
    {
        uint8_t OSIZE : 2; //!<  Digital Output Audio Sample Precision (0=16 bits, 1=20 bits, 2=24 bits, 3=8bits).
        uint8_t OMONO : 1; //!<  Digital Output Mono Mode (0=Use mono/stereo blend ).
        uint8_t OMODE : 4; //!<  Digital Output Mode (0000=I2S, 0110 = Left-justified, 1000 = MSB at second DCLK after DFS pulse, 1100 = MSB at first DCLK after DFS pulse).
        uint8_t OFALL : 1; //!<  Digital Output DCLK Edge (0 = use DCLK rising edge, 1 = use DCLK falling edge)
        uint8_t dummy : 8; //!<  Always 0.
    } refined;
    uint16_t raw;
} si4735_digital_output_format;

/**
 * @ingroup group03
 * 
 * @brief Digital audio output sample structure (Property 0x0104. DIGITAL_OUTPUT_SAMPLE_RATE).
 * 
 * @details Used to enable digital audio output and to configure the digital audio output sample rate in samples per second (sps).
 * 
 * @see Si47XX PROGRAMMING GUIDE; AN332; page 196. 
 */
typedef struct
{
    uint16_t DOSR;                   // Digital Output Sample Rate(32–48 ksps .0 to disable digital audio output).
} si4735_digital_output_sample_rate; // Maybe not necessary

volatile static bool data_from_si4735; /** @ingroup group04 store the interrupt status */

/**
 * @brief Interrupt Function
 * 
 * @details this function just set the volatile static bool data_from_si4735 to true;
 * 
 * If you are using interrupt feature, this function will be called by the system, not by you. 
 * If you are not using interrupt feature, please, ignore the compile message:
 * "warning: 'void interrupt_hundler()' defined but not used [-Wunused-function]"
 */
static void interrupt_hundler()
{
    data_from_si4735 = true;
};

/********************************************************************** 
 * SI4735 Class definition
 **********************************************************************/

/**
 * @brief SI4735 Class 
 * 
 * @details This class implements all functions to help you to control the Si47XX devices. 
 * This library was built based on “Si47XX PROGRAMMING GUIDE; AN332 ”. 
 * It also can be used on all members of the SI473X family respecting, of course, the features available 
 * for each IC version.  These functionalities can be seen in the comparison matrix shown in 
 * table 1 (Product Family Function); pages 2 and 3 of the programming guide.
 * 
 * @author PU2CLR - Ricardo Lima Caratti 
 */
class SI4735
{
protected:
    char rds_buffer2A[65]; //!<  RDS Radio Text buffer - Program Information
    char rds_buffer2B[33]; //!<  RDS Radio Text buffer - Station Informaation
    char rds_buffer0A[9];  //!<  RDS Basic tuning and switching information (Type 0 groups)
    char rds_time[20];     //!<  RDS date time received information

    int rdsTextAdress2A; //!<  rds_buffer2A current position
    int rdsTextAdress2B; //!<  rds_buffer2B current position
    int rdsTextAdress0A; //!<  rds_buffer0A current position

    int16_t deviceAddress = SI473X_ADDR_SEN_LOW; //!<  current I2C buss address

    uint16_t maxDelaySetFrequency = MAX_DELAY_AFTER_SET_FREQUENCY; // Max delay (ms) after set frequency command
    uint16_t maxDelayAfterPouwerUp = MAX_DELAY_AFTER_POWERUP;      //  Max delay you have to setup after a power up command.

    uint8_t lastTextFlagAB;
    uint8_t resetPin;     //!<  pin used on Arduino Board to RESET the Si47XX device
    uint8_t interruptPin; //!<  pin used on Arduino Board to control interrupt. If -1, interrupt is no used.

    uint8_t currentTune; //!<  tell the current tune (FM, AM or SSB)

    uint16_t currentMinimumFrequency; //!<  minimum frequency of the current band
    uint16_t currentMaximumFrequency; //!<  maximum frequency of the current band
    uint16_t currentWorkFrequency;    //!<  current frequency

    uint16_t currentStep; //!<  current steps

    uint8_t lastMode = -1; //!<  Store the last mode used.

    uint8_t currentAvcAmMaxGain = 48; //!<  Automatic Volume Control Gain for AM - Default 48

    si47x_frequency currentFrequency; //!<  data structure to get current frequency
    si47x_set_frequency currentFrequencyParams;
    si47x_rqs_status currentRqsStatus;       //!<  current Radio SIgnal Quality status
    si47x_response_status currentStatus;     //!<  current device status
    si47x_firmware_information firmwareInfo; //!<  firmware information
    si47x_rds_status currentRdsStatus;       //!<  current RDS status
    si47x_agc_status currentAgcStatus;       //!<  current AGC status
    si47x_ssb_mode currentSSBMode;           //!<  indicates if USB or LSB

    si473x_powerup powerUp;

    uint8_t volume = 32;

    uint8_t currentSsbStatus;

    void waitInterrupr(void);
    void sendProperty(uint16_t propertyNumber, uint16_t param);

    void sendSSBModeProperty();
    void disableFmDebug();
    void clearRdsBuffer2A();
    void clearRdsBuffer2B();
    void clearRdsBuffer0A();

public:
    SI4735();
    void reset(void);
    void waitToSend(void);

    void setup(uint8_t resetPin, uint8_t defaultFunction);
    void setup(uint8_t resetPin, int interruptPin, uint8_t defaultFunction, uint8_t audioMode = SI473X_ANALOG_AUDIO);

    int32_t getProperty(uint16_t propertyValue);

    /**
     * @brief Sets the a given Property 
     * 
     * @details Sets the Si47XX device with a given attribute. 
     * @details You might need to use the bit operations or some bit field structure to set the right the values.
     * 
     * @see Si47XX PROGRAMMING GUIDE; AN332; pages 55, 69, 124 and  134.
     * @param propertyNumber
     * @param param  pamameter value 
     */
    inline void setProperty(uint16_t propertyNumber, uint16_t param) {
        sendProperty(propertyNumber, param);
    };


    void setPowerUp(uint8_t CTSIEN, uint8_t GPO2OEN, uint8_t PATCH, uint8_t XOSCEN, uint8_t FUNC, uint8_t OPMODE);
    void radioPowerUp(void);
    void analogPowerUp(void);
    void powerDown(void);

    void setFrequency(uint16_t);

    void getStatus(uint8_t, uint8_t);

    uint16_t getFrequency(void);

    /** 
     * STATUS RESPONSE
     * Set of methods to get current status information. Call them after getStatus or getFrequency or seekStation
     * @see Si47XX PROGRAMMING GUIDE; AN332; pages 63
     */

    /**
     * @brief Get the Signal Quality Interrupt status
     * @see Si47XX PROGRAMMING GUIDE; AN332; pages 63            
     * @return RDSINT status
     */
    inline bool getSignalQualityInterrupt()
    {
        return currentStatus.resp.RSQINT;
    };

    /**
     * @brief Get the Radio Data System (RDS) Interrupt status
     * @see Si47XX PROGRAMMING GUIDE; AN332; page 63
     * @return RDSINT status  
     */
    inline bool getRadioDataSystemInterrupt()
    {
        return currentStatus.resp.RDSINT;
    };

    /**
     * @brief Get the Tune Complete status
     * @details Seek/Tune Complete Interrupt; 1 = Tune complete has been triggered.
     * @return STCINT status  
     */
    inline bool getTuneCompleteTriggered()
    {
        return currentStatus.resp.STCINT;
    };

    /**
     * @brief Get the Status Error 
     * @details Return the Error flag (true or false) of status of the least Tune or Seek
     * @return Error flag 
     */
    inline bool getStatusError()
    {
        return currentStatus.resp.ERR;
    };

    /**
     * @brief Gets the Error flag Clear to Send
     * 
     * @return CTS 
     */
    inline bool getStatusCTS() { return currentStatus.resp.CTS; };

    /**
     * @brief Returns true if the AFC rails (AFC Rail Indicator).
     * 
     * @return true 
     */
    inline bool getACFIndicator()
    {
        return currentStatus.resp.AFCRL;
    };

    /**
     * @brief Returns true if a seek hit the band limit 
     * 
     * @details (WRAP = 0 in FM_START_SEEK) or wrapped to the original frequency(WRAP = 1).
     * 
     * @return BLTF 
     */
    inline bool getBandLimit()
    {
        return currentStatus.resp.BLTF;
    };

    /**
     * @brief Gets the channel status
     * 
     * @details Returns true if the channel is currently valid as determined by the seek/tune properties (0x1403, 0x1404, 0x1108)
     * 
     * @return true 
     * @return false 
     */
    inline bool getStatusValid()
    {
        return currentStatus.resp.VALID;
    };

    /**
     * @brief Returns the value of  Received Signal Strength Indicator (dBμV).
     * 
     * @return uint8_t 
     */
    inline uint8_t getReceivedSignalStrengthIndicator()
    {
        return currentStatus.resp.RSSI;
    };

    /**
     * @brief Gets the SNR metric when tune is complete (dB)
     * 
     * @details Returns the value  of the SNR metric when tune is complete (dB).
     * 
     * @return uint8_t 
     */
    inline uint8_t getStatusSNR()
    {
        return currentStatus.resp.SNR;
    };

    /**
     * @brief Get the Status the M U L T 
     * 
     * @details Returns the value containing the multipath metric when tune is complete.
     * 
     * @return uint8_t 
     */
    inline uint8_t getStatusMULT()
    {
        return currentStatus.resp.MULT;
    };

    /**
     * @brief Get the Antenna Tuning Capacitor value
     * 
     * @details Returns the current antenna tuning capacitor value.
     * 
     * @return uint8_t capacitance 
     */
    inline uint8_t getAntennaTuningCapacitor()
    {
        return currentStatus.resp.READANTCAP;
    };

    void getAutomaticGainControl(); //!<  Queries Automatic Gain Control STATUS

    /**
     * @brief Sets the Avc Am Max Gain to 48dB
     * 
     */
    inline void setAvcAmMaxGain()
    {
        sendProperty(AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN, ((currentAvcAmMaxGain = 48) * 340));
    };

    void setAvcAmMaxGain(uint8_t gain); //!<  Sets the maximum gain for automatic volume control.

    /**
     * @brief Get the current Avc Am Max Gain 
     * 
     * @return uint8_t Current AVC gain index value
     */
    inline uint8_t getCurrentAvcAmMaxGain()
    {
        return currentAvcAmMaxGain;
    };

    /**
     * @brief Sets the Am Soft Mute Max Attenuation 
     * 
     * @details This function can be useful to disable Soft Mute. The value 0 disable soft mute.
     * @details Specified in units of dB. Default maximum attenuation is 8 dB.
     * 
     * @see Si47XX PROGRAMMING GUIDE; AN332; page 158. 
     * 
     * @param smattn Maximum attenuation to apply when in soft mute
     */
    inline void setAmSoftMuteMaxAttenuation(uint8_t smattn = 0)
    {
        sendProperty(AM_SOFT_MUTE_MAX_ATTENUATION, smattn);
    };

    /**
     * @brief Sets the SSB Soft Mute Max Attenuation object
     * 
     * @details Sets maximum attenuation during soft mute (dB). Set to 0 to disable soft mute. 
     * @details Specified in units of dB. Default maximum attenuation is 8 dB.
     * 
     * @param smattn Maximum attenuation to apply when in soft mute.
     */
    inline void setSsbSoftMuteMaxAttenuation(uint8_t smattn = 0)
    {
        sendProperty(SSB_SOFT_MUTE_MAX_ATTENUATION, smattn);
    };

    /**
     * @brief Checks if the AGC is enabled
     *
     * @return true if the AGC is enabled 
     */
    inline bool isAgcEnabled()
    {
        return !currentAgcStatus.refined.AGCDIS;
    };

    /**
     * @brief Gets the current AGC gain index
     * 
     * @return uint8_t The current AGC gain index.
     */
    inline uint8_t getAgcGainIndex()
    {
        return currentAgcStatus.refined.AGCIDX;
    };

    void setAutomaticGainControl(uint8_t AGCDIS, uint8_t AGCIDX);

    void getCurrentReceivedSignalQuality(uint8_t INTACK);
    void getCurrentReceivedSignalQuality(void);

    // AM and FM

    /**
     * @brief Get the current receive signal strength (0–127 dBμV)
     * 
     * @return uint8_t a value between 0 to 127
     */
    inline uint8_t getCurrentRSSI()
    {
        return currentRqsStatus.resp.RSSI;
    };

    /**
     * @brief Gets the current SNR metric (0–127 dB).
     * 
     * @return uint8_t SNR value in dB (0-127)
     */
    inline uint8_t getCurrentSNR()
    {
        return currentRqsStatus.resp.SNR;
    };

    /**
     * @brief Checks if RSSI detected is LOW. 
     * 
     * @return true if RSSI is low
     */
    inline bool getCurrentRssiDetectLow()
    {
        return currentRqsStatus.resp.RSSIILINT;
    };

    /**
     * @brief Checks if RSSI detected is high 
     * 
     * @return true if RSSI detected is high
     */
    inline bool getCurrentRssiDetectHigh()
    {
        return currentRqsStatus.resp.RSSIHINT;
    };

    /**
     * @brief Checks if SNR detect is low
     * 
     * @return true if SNR detected is low
     */
    inline bool getCurrentSnrDetectLow()
    {
        return currentRqsStatus.resp.SNRLINT;
    };

    /**
     * @brief Checks if SNR detect is high
     * 
     * @return true  if SNR detect is high
     */
    inline bool getCurrentSnrDetectHigh()
    {
        return currentRqsStatus.resp.SNRHINT;
    };

    /**
     * @brief Checks if the current channel is valid
     * 
     * @return true if the current channel is valid
     */
    inline bool getCurrentValidChannel()
    {
        return currentRqsStatus.resp.VALID;
    };

    /**
     * @brief AFC Rail Indicator
     * 
     * @return true or false
     */
    inline bool getCurrentAfcRailIndicator()
    {
        return currentRqsStatus.resp.AFCRL;
    };

    /**
     * @brief Soft Mute Indicator. 
     * 
     * @details Indicates soft mute is engaged.
     * 
     * @return true  if soft mute indicates is engaged.
     */
    inline bool getCurrentSoftMuteIndicator()
    {
        return currentRqsStatus.resp.SMUTE;
    };

    // Just FM

    /**
     * @brief Gets the value of the amount of stereo blend in % (100 = full stereo, 0 = full mono).
     * 
     * @return uint8_t value (0 to 100)
     */
    inline uint8_t getCurrentStereoBlend()
    {
        return currentRqsStatus.resp.STBLEND;
    };

    /**
     * @brief Checks the current pilot 
     * 
     * @details Indicates stereo pilot presence.
     * 
     * @return true if stereo pilot presence has detected
     */
    inline bool getCurrentPilot()
    {
        return currentRqsStatus.resp.PILOT;
    };

    /**
     * @brief Gets the current Multipath
     *  
     * @details Contains the current multipath metric. (0 = no multipath; 100 = full multipath)
     * 
     * @return uint8_t value (0 to 100)
     */
    inline uint8_t getCurrentMultipath()
    {
        return currentRqsStatus.resp.MULT;
    };

    inline uint8_t getCurrentSignedFrequencyOffset() { return currentRqsStatus.resp.FREQOFF; }; //!<  Signed frequency offset (kHz).
    inline bool getCurrentMultipathDetectLow() { return currentRqsStatus.resp.MULTLINT; };      //!<  Multipath Detect Low.
    inline bool getCurrentMultipathDetectHigh() { return currentRqsStatus.resp.MULTHINT; };     //!<  Multipath Detect High
    inline bool getCurrentBlendDetectInterrupt() { return currentRqsStatus.resp.BLENDINT; };    //!<  Blend Detect Interrupt

    /*
     * FIRMWARE RESPONSE
     * 
     * See Si47XX PROGRAMMING GUIDE; AN332; page 66
     */

    inline uint8_t getFirmwarePN() { return firmwareInfo.resp.PN; };             //!<   RESP1 - Part Number (HEX)
    inline uint8_t getFirmwareFWMAJOR() { return firmwareInfo.resp.FWMAJOR; };   //!<  RESP2 - Returns the Firmware Major Revision (ASCII).
    inline uint8_t getFirmwareFWMINOR() { return firmwareInfo.resp.FWMINOR; };   //!<  RESP3 - Returns the Firmware Minor Revision (ASCII).
    inline uint8_t getFirmwarePATCHH() { return firmwareInfo.resp.PATCHH; };     //!<  RESP4 -  Returns the Patch ID High byte (HEX).
    inline uint8_t getFirmwarePATCHL() { return firmwareInfo.resp.PATCHL; };     //!<  RESP5 - Returns the Patch ID Low byte (HEX).
    inline uint8_t getFirmwareCMPMAJOR() { return firmwareInfo.resp.CMPMAJOR; }; //!<  RESP6 -  Returns the Component Major Revision (ASCII).
    inline uint8_t getFirmwareCMPMINOR() { return firmwareInfo.resp.CMPMINOR; }; //!<  RESP7 - Returns the Component Minor Revision (ASCII).
    inline uint8_t getFirmwareCHIPREV() { return firmwareInfo.resp.CHIPREV; };   //!<  RESP8 -  Returns the Chip Revision (ASCII).

    void setVolume(uint8_t volume);
    uint8_t getVolume();
    void volumeDown();
    void volumeUp();
    inline uint8_t getCurrentVolume() { return volume; }; //!<  Returns the current volume level.
    void setAudioMute(bool off);                          // if true mute the audio; else unmute

    void digitalOutputFormat(uint8_t OSIZE, uint8_t OMONO, uint8_t OMODE, uint8_t OFALL);
    void digitalOutputSampleRate(uint16_t DOSR);

    void setAM();
    void setFM();
    void setAM(uint16_t fromFreq, uint16_t toFreq, uint16_t intialFreq, uint16_t step);
    void setFM(uint16_t fromFreq, uint16_t toFreq, uint16_t initialFreq, uint16_t step);

    void setBandwidth(uint8_t AMCHFLT, uint8_t AMPLFLT);

    inline uint8_t getTuneFrequencyFast() { return currentFrequencyParams.arg.FAST; };                  //!<  Returns the FAST tuning status
    inline void setTuneFrequencyFast(uint8_t FAST) { currentFrequencyParams.arg.FAST = FAST; };         //!<  FAST Tuning.  If set, executes fast and invalidated tune. The tune status will not be accurate
    inline uint8_t getTuneFrequencyFreeze() { return currentFrequencyParams.arg.FREEZE; };              //!<  Returns the FREEZE status
    inline void setTuneFrequencyFreeze(uint8_t FREEZE) { currentFrequencyParams.arg.FREEZE = FREEZE; }; //!<  Only FM. Freeze Metrics During Alternate Frequency Jump.
    void setTuneFrequencyAntennaCapacitor(uint16_t capacitor);

    void frequencyUp();
    void frequencyDown();
    bool isCurrentTuneFM();
    bool isCurrentTuneAM();
    bool isCurrentTuneSSB();

    void getFirmware(void);

    void seekStation(uint8_t SEEKUP, uint8_t WRAP);
    void seekStationUp();
    void seekStationDown();
    void setSeekAmLimits(uint16_t bottom, uint16_t top);
    void setSeekAmSpacing(uint16_t spacing);
    void setSeekSrnThreshold(uint16_t value);
    void setSeekRssiThreshold(uint16_t value);

    void setFmBlendStereoThreshold(uint8_t parameter);
    void setFmBlendMonoThreshold(uint8_t parameter);
    void setFmBlendRssiStereoThreshold(uint8_t parameter);
    void setFmBLendRssiMonoThreshold(uint8_t parameter);
    void setFmBlendSnrStereoThreshold(uint8_t parameter);
    void setFmBLendSnrMonoThreshold(uint8_t parameter);
    void setFmBlendMultiPathStereoThreshold(uint8_t parameter);
    void setFmBlendMultiPathMonoThreshold(uint8_t parameter);
    void setFmStereoOn();
    void setFmStereoOff();

    void RdsInit();
    void setRdsIntSource(uint8_t RDSNEWBLOCKB, uint8_t RDSNEWBLOCKA, uint8_t RDSSYNCFOUND, uint8_t RDSSYNCLOST, uint8_t RDSRECV);
    void getRdsStatus(uint8_t INTACK, uint8_t MTFIFO, uint8_t STATUSONLY);
    void getRdsStatus();

    /**
     * @brief Get the Rds Received FIFO
     * @details if FIFO is 1, it means the minimum number of groups was filled
     * @return true if minimum number of groups was filled.
     */
    inline bool getRdsReceived()
    {
        return currentRdsStatus.resp.RDSRECV;
    };

    /**
     * @brief Get the Rds Sync Lost object
     * @details returns true (1) if Lost RDS synchronization is detected. 
     * @return true if Lost RDS synchronization detected. 
     */
    inline bool getRdsSyncLost()
    {
        return currentRdsStatus.resp.RDSSYNCLOST;
    };

    /**
     * @brief Get the Rds Sync Found 
     * @details return true if found RDS synchronization
     * @return true if found RDS synchronization
     */
    inline bool getRdsSyncFound()
    {
        return currentRdsStatus.resp.RDSSYNCFOUND;
    };

    /**
     * @brief Get the Rds New Block A 
     * 
     * @details Returns true if valid Block A data has been received.
     * @return true or false
     */
    inline bool getRdsNewBlockA()
    {
        return currentRdsStatus.resp.RDSNEWBLOCKA;
    };

    /**
     * @brief Get the Rds New Block B 
     * @details Returns true if valid Block B data has been received.
     * @return true or false
     */
    inline bool getRdsNewBlockB()
    {
        return currentRdsStatus.resp.RDSNEWBLOCKB;
    };

    /**
     * @brief Get the Rds Sync 
     * @details Returns true if RDS currently synchronized.
     * @return true or false
     */
    inline bool getRdsSync()
    {
        return currentRdsStatus.resp.RDSSYNC;
    };

    /**
     * @brief Get the Group Lost 
     * @details Returns true if one or more RDS groups discarded due to FIFO overrun.
     * @return true or false
     */
    inline bool getGroupLost()
    {
        return currentRdsStatus.resp.GRPLOST;
    };

    /**
     * @brief Get the Num Rds Fifo Used 
     * @details Return the number of RDS FIFO used
     * @return uint8_t Total RDS FIFO used
     */
    inline uint8_t getNumRdsFifoUsed()
    {
        return currentRdsStatus.resp.RDSFIFOUSED;
    };

    void setRdsConfig(uint8_t RDSEN, uint8_t BLETHA, uint8_t BLETHB, uint8_t BLETHC, uint8_t BLETHD);
    uint16_t getRdsPI(void);
    uint8_t getRdsGroupType(void);
    uint8_t getRdsFlagAB(void);
    uint8_t getRdsVersionCode(void);
    uint8_t getRdsProgramType(void);
    uint8_t getRdsTextSegmentAddress(void);

    char *getRdsText(void);
    char *getRdsText0A(void); // Gets the Station name
    char *getRdsText2A(void); // Gets the Radio Text
    char *getRdsText2B(void);

    char *getRdsTime(void);

    void getNext2Block(char *);
    void getNext4Block(char *);

    void ssbSetup();
    void setSSBBfo(int offset);
    void setSSBConfig(uint8_t AUDIOBW, uint8_t SBCUTFLT, uint8_t AVC_DIVIDER, uint8_t AVCEN, uint8_t SMUTESEL, uint8_t DSP_AFCDIS);
    void setSSB(uint16_t fromFreq, uint16_t toFreq, uint16_t intialFreq, uint16_t step, uint8_t usblsb);
    void setSSB(uint8_t usblsb);
    void setSSBAudioBandwidth(uint8_t AUDIOBW);
    void setSSBAutomaticVolumeControl(uint8_t AVCEN);
    void setSBBSidebandCutoffFilter(uint8_t SBCUTFLT);
    void setSSBAvcDivider(uint8_t AVC_DIVIDER);
    void setSSBDspAfc(uint8_t DSP_AFCDIS);
    void setSSBSoftMute(uint8_t SMUTESEL);

    si47x_firmware_query_library queryLibraryId();
    void patchPowerUp();
    bool downloadPatch(const uint8_t *ssb_patch_content, const uint16_t ssb_patch_content_size);
    bool downloadPatch(int eeprom_i2c_address);
    void ssbPowerUp();

    /** 
     * This functions below modify the clock frequency for I2C communication. 
     * 100KHz  is usually the baseline.
     * Use one of these funcition if you have problem on you default configuration. 
     */

    /**
     * @brief Sets I2C buss to 10KHz
     */
    inline void setI2CLowSpeedMode(void)
    {
        Wire.setClock(10000);
    };

    /**
     * @brief Sets I2C buss to 100KHz
     */
    inline void setI2CStandardMode(void) { Wire.setClock(100000); };

    /**
     * @brief Sets I2C buss to 400KHz
     */
    inline void setI2CFastMode(void)
    {
        Wire.setClock(400000);
    };

    /**
     * Sets the I2C bus to a given value.
     * 
     * ATTENTION: use this function with cation
     * 
     * @param value in Hz. For example: The values 500000 sets the bus to 500KHz.
     */
    inline void setI2CFastModeCustom(long value = 500000) { Wire.setClock(value); };

    /**
     * @ingroup group06 Si47XX device Power Up 
     * @brief Set the Max Delay Power Up 
     * @details Sets the delay needed in ms after a powerup command (default is 10ms).
     * @details Some external crystal might need more time to become stable (500 ms is the recommended).
     * @details Low values make the load SSB patch faster. However, it can make the system unstable.   
     * 
     * @see MAX_DELAY_AFTER_POWERUP  
     * @param ms delay in ms
     */
    inline void setMaxDelayPowerUp(uint16_t ms)
    {
        this->maxDelayAfterPouwerUp = ms;
    }

    /**
     * @ingroup   group08 Tune Frequency
     * @brief Set the Max Delay after Set Frequency 
     * 
     * @details After the set frequency command, the system need a time to get ready to the next set frequency (default value 30ms).
     * @details Why the waitToSend() does not work in this case? No idea for while! 
     * @details A low value makes the getFrequency command inaccurate. 
     * 
     * @see  MAX_DELAY_AFTER_POWERUP
     * @param ms 
     */
    inline void setMaxDelaySetFrequency(uint16_t ms)
    {
        this->maxDelaySetFrequency = ms;
    }

    /** 
     * @ingroup group08 Tune Frequency step
     * 
     * @brief Sets the current step value. 
     * 
     * @details This function does not check the limits of the current band. Please, don't take a step bigger than your legs.
     * @details Example:
     * @code
     * setFM(6400,10800,10390,10);
     * setFrequencyStep(100); // the step will be 1MHz (you are using FM mode)
     * .
     * .
     * .
     * setAM(7000,7600,7100,5); 
     * setFrequencyStep(1); // the step will be 1KHz (you are usin AM or SSB mode)  
     * @endcode 
     * 
     * @see setFM()
     * @see setAM()
     * @see setSSB()
     * 
     * @param step if you are using FM, 10 means 100KHz. If you are using AM 10 means 10KHz
     *             For AM, 1 (1KHz) to 1000 (1MHz) are valid values.
     *             For FM 5 (50KHz), 10 (100KHz) and 100 (1MHz) are valid values.  
     */
    inline void setFrequencyStep(uint16_t step)
    {
        this->currentStep = step;
    }

    /**
     * @ingroup group14 Frequency 
     * 
     * @brief Gets the current frequency saved in memory. 
     * 
     * @details Unlike getFrequency, this method gets the current frequency recorded after the last setFrequency command. 
     * @details This method avoids bus traffic and CI processing.
     * @details However, you can not get others status information like RSSI.
     * 
     * @see getFrequency()
     */
    inline uint16_t getCurrentFrequency()
    {
        return this->currentWorkFrequency;
    }

    /**
     * @ingroup group14 Si47XX device Status 
     * 
     * @brief Gets the current status  of the Si47XX (AM, FM or SSB)
     * 
     * @see Si47XX PROGRAMMING GUIDE; AN332; pages 73 (FM) and 139 (AM)
     */
    inline void getStatus()
    {
        getStatus(0, 1);
    }

    void setDeviceI2CAddress(uint8_t senPin);
    int16_t getDeviceI2CAddress(uint8_t resetPin);
    void setDeviceOtherI2CAddress(uint8_t i2cAddr);
};
