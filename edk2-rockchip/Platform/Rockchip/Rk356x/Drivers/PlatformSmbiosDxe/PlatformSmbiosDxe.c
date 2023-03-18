/** @file
 *
 *  Static SMBIOS Table for RK3566/RK3568 based platforms
 *  Derived from the RaspberryPi package.
 *
 *  Note - Arm SBBR ver 1.2 required and recommended SMBIOS structures:
 *    BIOS Information (Type 0)
 *    System Information (Type 1)
 *    Board Information (Type 2) - Recommended
 *    System Enclosure (Type 3)
 *    Processor Information (Type 4) - CPU Driver
 *    Cache Information (Type 7) - For cache that is external to processor
 *    Port Information (Type 8) - Recommended for platforms with physical ports
 *    System Slots (Type 9) - If system has slots
 *    OEM Strings (Type 11) - Recommended
 *    BIOS Language Information (Type 13) - Recommended
 *    System Event Log (Type 15) - Recommended (does not exit on RPi)
 *    Physical Memory Array (Type 16)
 *    Memory Device (Type 17) - For each socketed system-memory Device
 *    Memory Array Mapped Address (Type 19) - One per contiguous block per Physical Memroy Array
 *    System Boot Information (Type 32)
 *    IPMI Device Information (Type 38) - Required for platforms with IPMIv1.0 BMC Host Interface (not applicable to RPi)
 *    Onboard Devices Extended Information (Type 41) - Recommended
 *    Redfish Host Interface (Type 42) - Required for platforms supporting Redfish Host Interface (not applicable to RPi)
 *
 *  Copyright (c) 2017-2021, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) 2013, Linaro.org
 *  Copyright (c) 2012, Apple Inc. All rights reserved.<BR>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Base.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>
#include <Guid/SmBios.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/CruLib.h>
#include <Library/SdramLib.h>
#include <Library/OtpLib.h>
#include <Protocol/ArmScmiClockProtocol.h>

#define SMB_IS_DIGIT(c)  (((c) >= '0') && ((c) <= '9'))

STATIC UINT64 mMemorySize = 0;

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT32  mCrcTable[256] = {
  0x00000000,
  0x77073096,
  0xEE0E612C,
  0x990951BA,
  0x076DC419,
  0x706AF48F,
  0xE963A535,
  0x9E6495A3,
  0x0EDB8832,
  0x79DCB8A4,
  0xE0D5E91E,
  0x97D2D988,
  0x09B64C2B,
  0x7EB17CBD,
  0xE7B82D07,
  0x90BF1D91,
  0x1DB71064,
  0x6AB020F2,
  0xF3B97148,
  0x84BE41DE,
  0x1ADAD47D,
  0x6DDDE4EB,
  0xF4D4B551,
  0x83D385C7,
  0x136C9856,
  0x646BA8C0,
  0xFD62F97A,
  0x8A65C9EC,
  0x14015C4F,
  0x63066CD9,
  0xFA0F3D63,
  0x8D080DF5,
  0x3B6E20C8,
  0x4C69105E,
  0xD56041E4,
  0xA2677172,
  0x3C03E4D1,
  0x4B04D447,
  0xD20D85FD,
  0xA50AB56B,
  0x35B5A8FA,
  0x42B2986C,
  0xDBBBC9D6,
  0xACBCF940,
  0x32D86CE3,
  0x45DF5C75,
  0xDCD60DCF,
  0xABD13D59,
  0x26D930AC,
  0x51DE003A,
  0xC8D75180,
  0xBFD06116,
  0x21B4F4B5,
  0x56B3C423,
  0xCFBA9599,
  0xB8BDA50F,
  0x2802B89E,
  0x5F058808,
  0xC60CD9B2,
  0xB10BE924,
  0x2F6F7C87,
  0x58684C11,
  0xC1611DAB,
  0xB6662D3D,
  0x76DC4190,
  0x01DB7106,
  0x98D220BC,
  0xEFD5102A,
  0x71B18589,
  0x06B6B51F,
  0x9FBFE4A5,
  0xE8B8D433,
  0x7807C9A2,
  0x0F00F934,
  0x9609A88E,
  0xE10E9818,
  0x7F6A0DBB,
  0x086D3D2D,
  0x91646C97,
  0xE6635C01,
  0x6B6B51F4,
  0x1C6C6162,
  0x856530D8,
  0xF262004E,
  0x6C0695ED,
  0x1B01A57B,
  0x8208F4C1,
  0xF50FC457,
  0x65B0D9C6,
  0x12B7E950,
  0x8BBEB8EA,
  0xFCB9887C,
  0x62DD1DDF,
  0x15DA2D49,
  0x8CD37CF3,
  0xFBD44C65,
  0x4DB26158,
  0x3AB551CE,
  0xA3BC0074,
  0xD4BB30E2,
  0x4ADFA541,
  0x3DD895D7,
  0xA4D1C46D,
  0xD3D6F4FB,
  0x4369E96A,
  0x346ED9FC,
  0xAD678846,
  0xDA60B8D0,
  0x44042D73,
  0x33031DE5,
  0xAA0A4C5F,
  0xDD0D7CC9,
  0x5005713C,
  0x270241AA,
  0xBE0B1010,
  0xC90C2086,
  0x5768B525,
  0x206F85B3,
  0xB966D409,
  0xCE61E49F,
  0x5EDEF90E,
  0x29D9C998,
  0xB0D09822,
  0xC7D7A8B4,
  0x59B33D17,
  0x2EB40D81,
  0xB7BD5C3B,
  0xC0BA6CAD,
  0xEDB88320,
  0x9ABFB3B6,
  0x03B6E20C,
  0x74B1D29A,
  0xEAD54739,
  0x9DD277AF,
  0x04DB2615,
  0x73DC1683,
  0xE3630B12,
  0x94643B84,
  0x0D6D6A3E,
  0x7A6A5AA8,
  0xE40ECF0B,
  0x9309FF9D,
  0x0A00AE27,
  0x7D079EB1,
  0xF00F9344,
  0x8708A3D2,
  0x1E01F268,
  0x6906C2FE,
  0xF762575D,
  0x806567CB,
  0x196C3671,
  0x6E6B06E7,
  0xFED41B76,
  0x89D32BE0,
  0x10DA7A5A,
  0x67DD4ACC,
  0xF9B9DF6F,
  0x8EBEEFF9,
  0x17B7BE43,
  0x60B08ED5,
  0xD6D6A3E8,
  0xA1D1937E,
  0x38D8C2C4,
  0x4FDFF252,
  0xD1BB67F1,
  0xA6BC5767,
  0x3FB506DD,
  0x48B2364B,
  0xD80D2BDA,
  0xAF0A1B4C,
  0x36034AF6,
  0x41047A60,
  0xDF60EFC3,
  0xA867DF55,
  0x316E8EEF,
  0x4669BE79,
  0xCB61B38C,
  0xBC66831A,
  0x256FD2A0,
  0x5268E236,
  0xCC0C7795,
  0xBB0B4703,
  0x220216B9,
  0x5505262F,
  0xC5BA3BBE,
  0xB2BD0B28,
  0x2BB45A92,
  0x5CB36A04,
  0xC2D7FFA7,
  0xB5D0CF31,
  0x2CD99E8B,
  0x5BDEAE1D,
  0x9B64C2B0,
  0xEC63F226,
  0x756AA39C,
  0x026D930A,
  0x9C0906A9,
  0xEB0E363F,
  0x72076785,
  0x05005713,
  0x95BF4A82,
  0xE2B87A14,
  0x7BB12BAE,
  0x0CB61B38,
  0x92D28E9B,
  0xE5D5BE0D,
  0x7CDCEFB7,
  0x0BDBDF21,
  0x86D3D2D4,
  0xF1D4E242,
  0x68DDB3F8,
  0x1FDA836E,
  0x81BE16CD,
  0xF6B9265B,
  0x6FB077E1,
  0x18B74777,
  0x88085AE6,
  0xFF0F6A70,
  0x66063BCA,
  0x11010B5C,
  0x8F659EFF,
  0xF862AE69,
  0x616BFFD3,
  0x166CCF45,
  0xA00AE278,
  0xD70DD2EE,
  0x4E048354,
  0x3903B3C2,
  0xA7672661,
  0xD06016F7,
  0x4969474D,
  0x3E6E77DB,
  0xAED16A4A,
  0xD9D65ADC,
  0x40DF0B66,
  0x37D83BF0,
  0xA9BCAE53,
  0xDEBB9EC5,
  0x47B2CF7F,
  0x30B5FFE9,
  0xBDBDF21C,
  0xCABAC28A,
  0x53B39330,
  0x24B4A3A6,
  0xBAD03605,
  0xCDD70693,
  0x54DE5729,
  0x23D967BF,
  0xB3667A2E,
  0xC4614AB8,
  0x5D681B02,
  0x2A6F2B94,
  0xB40BBE37,
  0xC30C8EA1,
  0x5A05DF1B,
  0x2D02EF8D
};

UINT32
EFIAPI
CalculateCrc32NoComp(
  IN  UINT32                       Crc,
  IN  VOID                         *Buffer,
  IN  UINTN                        Length
  )
{
  UINTN   Index;
  UINT8   *Ptr;

  ASSERT (Buffer != NULL);
  ASSERT (Length <= (MAX_ADDRESS - ((UINTN) Buffer) + 1));

  //
  // Compute CRC
  //
  for (Index = 0, Ptr = Buffer; Index < Length; Index++, Ptr++) {
    Crc = (Crc >> 8) ^ mCrcTable[(UINT8) Crc ^ *Ptr];
  }

  return Crc;
}


/***********************************************************************
        SMBIOS data definition  TYPE0  BIOS Information
************************************************************************/
SMBIOS_TABLE_TYPE0 mBIOSInfoType0 = {
  { EFI_SMBIOS_TYPE_BIOS_INFORMATION, sizeof (SMBIOS_TABLE_TYPE0), 0 },
  1,                                                     // Vendor String
  2,                                                     // BiosVersion String
  (UINT16) (FixedPcdGet32 (PcdFdBaseAddress) / 0x10000), // BiosSegment
  3,                                                     // BiosReleaseDate String
  (UINT8) (FixedPcdGet32 (PcdFdSize) / 0x10000),         // BiosSize (in 64KB)
  {                     // BiosCharacteristics
    0,    //  Reserved                          :2;  ///< Bits 0-1.
    0,    //  Unknown                           :1;
    0,    //  BiosCharacteristicsNotSupported   :1;
    0,    //  IsaIsSupported                    :1;
    0,    //  McaIsSupported                    :1;
    0,    //  EisaIsSupported                   :1;
    0,    //  PciIsSupported                    :1; /// No PCIe support since we hide ECAM from the OS
    0,    //  PcmciaIsSupported                 :1;
    0,    //  PlugAndPlayIsSupported            :1;
    0,    //  ApmIsSupported                    :1;
    1,    //  BiosIsUpgradable                  :1;
    0,    //  BiosShadowingAllowed              :1;
    0,    //  VlVesaIsSupported                 :1;
    0,    //  EscdSupportIsAvailable            :1;
    0,    //  BootFromCdIsSupported             :1;
    1,    //  SelectableBootIsSupported         :1;
    0,    //  RomBiosIsSocketed                 :1;
    0,    //  BootFromPcmciaIsSupported         :1;
    0,    //  EDDSpecificationIsSupported       :1;
    0,    //  JapaneseNecFloppyIsSupported      :1;
    0,    //  JapaneseToshibaFloppyIsSupported  :1;
    0,    //  Floppy525_360IsSupported          :1;
    0,    //  Floppy525_12IsSupported           :1;
    0,    //  Floppy35_720IsSupported           :1;
    0,    //  Floppy35_288IsSupported           :1;
    0,    //  PrintScreenIsSupported            :1;
    0,    //  Keyboard8042IsSupported           :1;
    0,    //  SerialIsSupported                 :1;
    0,    //  PrinterIsSupported                :1;
    0,    //  CgaMonoIsSupported                :1;
    0,    //  NecPc98                           :1;
    0     //  ReservedForVendor                 :32; ///< Bits 32-63. Bits 32-47 reserved for BIOS vendor
    ///< and bits 48-63 reserved for System Vendor.
  },
  {       // BIOSCharacteristicsExtensionBytes[]
    0x01, //  AcpiIsSupported                   :1;
          //  UsbLegacyIsSupported              :1;
          //  AgpIsSupported                    :1;
          //  I2OBootIsSupported                :1;
          //  Ls120BootIsSupported              :1;
          //  AtapiZipDriveBootIsSupported      :1;
          //  Boot1394IsSupported               :1;
          //  SmartBatteryIsSupported           :1;
          //  BIOSCharacteristicsExtensionBytes[1]
    0x0c, //  BiosBootSpecIsSupported              :1;
          //  FunctionKeyNetworkBootIsSupported    :1;
          //  TargetContentDistributionEnabled     :1;
          //  UefiSpecificationSupported           :1;
          //  VirtualMachineSupported              :1;
          //  ExtensionByte2Reserved               :3;
  },
  0,                       // SystemBiosMajorRelease
  0,                       // SystemBiosMinorRelease
  0,                       // EmbeddedControllerFirmwareMajorRelease
  0,                       // EmbeddedControllerFirmwareMinorRelease
  { (UINT16) (FixedPcdGet32 (PcdFdSize) / 0x100000) }, // BiosSize (in MB since Bits 15:14 = 00b)
};

CHAR8 mBiosVendor[128]  = "EDK2";
CHAR8 mBiosVersion[128] = "EDK2-DEV";
CHAR8 mBiosDate[12]     = "00/00/0000";

CHAR8 *mBIOSInfoType0Strings[] = {
  mBiosVendor,              // Vendor
  mBiosVersion,             // Version
  mBiosDate,                // Release Date
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE1  System Information
************************************************************************/
SMBIOS_TABLE_TYPE1 mSysInfoType1 = {
  { EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, sizeof (SMBIOS_TABLE_TYPE1), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  { 0x25EF0280, 0xEC82, 0x42B0, { 0x8F, 0xB6, 0x10, 0xAD, 0xCC, 0xC6, 0x7C, 0x02 } },
  SystemWakeupTypePowerSwitch,
  5,    // SKUNumber String
  6,    // Family String
};

CHAR8 mSysInfoManufName[128];
CHAR8 mSysInfoProductName[128];
CHAR8 mSysInfoFamilyName[128];
CHAR8 mSysInfoVersionName[128];
CHAR8 mSysInfoSerial[sizeof (UINT64) * 2 + 1];
CHAR8 mSysInfoSKU[sizeof (UINT64) * 2 + 1];

CHAR8 *mSysInfoType1Strings[] = {
  mSysInfoManufName,
  mSysInfoProductName,
  mSysInfoVersionName,
  mSysInfoSerial,
  mSysInfoSKU,
  mSysInfoFamilyName,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE2  Board Information
************************************************************************/
SMBIOS_TABLE_TYPE2 mBoardInfoType2 = {
  { EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, sizeof (SMBIOS_TABLE_TYPE2), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  5,    // AssetTag String
  {     // FeatureFlag
    1,    //  Motherboard           :1;
    0,    //  RequiresDaughterCard  :1;
    0,    //  Removable             :1;
    0,    //  Replaceable           :1;
    0,    //  HotSwappable          :1;
    0,    //  Reserved              :3;
  },
  6,                        // LocationInChassis String
  0,                        // ChassisHandle;
  BaseBoardTypeMotherBoard, // BoardType;
  0,                        // NumberOfContainedObjectHandles;
  { 0 }                     // ContainedObjectHandles[1];
};

CHAR8 mChassisAssetTag[128];

CHAR8 *mBoardInfoType2Strings[] = {
  mSysInfoManufName,
  mSysInfoProductName,
  mSysInfoVersionName,
  mSysInfoSerial,
  mChassisAssetTag,
  "Internal",
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE3  Enclosure Information
************************************************************************/
SMBIOS_TABLE_TYPE3 mEnclosureInfoType3 = {
  { EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, sizeof (SMBIOS_TABLE_TYPE3), 0 },
  1,                        // Manufacturer String
  MiscChassisEmbeddedPc,    // Type;
  2,                        // Version String
  3,                        // SerialNumber String
  4,                        // AssetTag String
  ChassisStateSafe,         // BootupState;
  ChassisStateSafe,         // PowerSupplyState;
  ChassisStateSafe,         // ThermalState;
  ChassisSecurityStatusNone,// SecurityStatus;
  { 0, 0, 0, 0 },           // OemDefined[4];
  0,    // Height;
  1,    // NumberofPowerCords;
  0,    // ContainedElementCount;
  0,    // ContainedElementRecordLength;
  { { 0 } },    // ContainedElements[1];
};
CHAR8 *mEnclosureInfoType3Strings[] = {
  mSysInfoManufName,
  mSysInfoProductName,
  mSysInfoSerial,
  mChassisAssetTag,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE4  Processor Information
************************************************************************/
SMBIOS_TABLE_TYPE4 mProcessorInfoType4 = {
  { EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof (SMBIOS_TABLE_TYPE4), 0},
  1,                               // Socket String
  CentralProcessor,                // ProcessorType;          ///< The enumeration value from PROCESSOR_TYPE_DATA.
  ProcessorFamilyIndicatorFamily2, // ProcessorFamily;        ///< The enumeration value from PROCESSOR_FAMILY2_DATA.
  2,                               // ProcessorManufacture String;
  {                                // ProcessorId;
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 }
  },
  3,                    // ProcessorVersion String;
  {                     // Voltage;
    1,  // ProcessorVoltageCapability5V        :1;
    1,  // ProcessorVoltageCapability3_3V      :1;
    1,  // ProcessorVoltageCapability2_9V      :1;
    0,  // ProcessorVoltageCapabilityReserved  :1; ///< Bit 3, must be zero.
    0,  // ProcessorVoltageReserved            :3; ///< Bits 4-6, must be zero.
    0   // ProcessorVoltageIndicateLegacy      :1;
  },
  0,                      // ExternalClock;
  0,                      // MaxSpeed;
  0,                      // CurrentSpeed;
  0x41,                   // Status;
  ProcessorUpgradeNone,   // ProcessorUpgrade;         ///< The enumeration value from PROCESSOR_UPGRADE.
  0xFFFF,                 // L1CacheHandle;
  0xFFFF,                 // L2CacheHandle;
  0xFFFF,                 // L3CacheHandle;
  0,                      // SerialNumber;
  0,                      // AssetTag;
  0,                      // PartNumber;
  4,                      // CoreCount;
  4,                      // EnabledCoreCount;
  4,                      // ThreadCount;
  0x6C,                   // ProcessorCharacteristics; ///< The enumeration value from PROCESSOR_CHARACTERISTIC_FLAGS
      // ProcessorReserved1              :1;
      // ProcessorUnknown                :1;
      // Processor64BitCapble            :1;
      // ProcessorMultiCore              :1;
      // ProcessorHardwareThread         :1;
      // ProcessorExecuteProtection      :1;
      // ProcessorEnhancedVirtualization :1;
      // ProcessorPowerPerformanceCtrl    :1;
      // Processor128bitCapble            :1;
      // ProcessorReserved2               :7;
  ProcessorFamilyARM,     // ARM Processor Family;
  0,                      // CoreCount2;
  0,                      // EnabledCoreCount2;
  0,                      // ThreadCount2;
};

CHAR8 mCpuName[128] = "Unknown ARM CPU";

CHAR8 *mProcessorInfoType4Strings[] = {
  "Socket",
  "Rockchip",
  mCpuName,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE7  Cache Information
************************************************************************/
SMBIOS_TABLE_TYPE7 mCacheInfoType7_L1I = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x380,                    // Cache Configuration
       //Cache Level        :3  (L1)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (Unknown)
       //Reserved           :6
  0x0020,                   // Maximum Size (32KB)
  0x0020,                   // Install Size (32KB)
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorParity,         // Error Correction
  CacheTypeInstruction,     // System Cache Type
  CacheAssociativity2Way    // Associativity  (RPi4 L1 Instruction cache is 3-way set associative, but SMBIOS spec does not define that)
};
CHAR8  *mCacheInfoType7Strings_L1I[] = {
  "L1 Instruction",
  NULL
};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_L1D = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x180,                    // Cache Configuration
       //Cache Level        :3  (L1)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (WB)
       //Reserved           :6
  0x0020,                   // Maximum Size (32KB)
  0x0020,                   // Install Size (32KB)
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorSingleBit,      // Error Correction
  CacheTypeData,            // System Cache Type
#if (RPI_MODEL == 4)
  CacheAssociativity2Way    // Associativity
#else
  CacheAssociativity4Way    // Associativity
#endif
};
CHAR8  *mCacheInfoType7Strings_L1D[] = {
  "L1 Data",
  NULL
};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_L2 = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x0181,                   // Cache Configuration
       //Cache Level        :3  (L2)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (WB)
       //Reserved           :6
  0x0200,                   // Maximum Size (512KB)
  0x0200,                   // Install Size (512KB)
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorSingleBit,      // Error Correction Multi
  CacheTypeUnified,         // System Cache Type
  CacheAssociativity16Way   // Associativity
};
CHAR8  *mCacheInfoType7Strings_L2[] = {
  "L2",
  NULL
};
/***********************************************************************
        SMBIOS data definition  TYPE9  System Slot Information
************************************************************************/
SMBIOS_TABLE_TYPE9  mSysSlotInfoType9 = {
  { EFI_SMBIOS_TYPE_SYSTEM_SLOTS, sizeof (SMBIOS_TABLE_TYPE9), 0 },
  1,    // SlotDesignation String
  SlotTypeOther,          // SlotType;                 ///< The enumeration value from MISC_SLOT_TYPE.
  SlotDataBusWidthOther,  // SlotDataBusWidth;         ///< The enumeration value from MISC_SLOT_DATA_BUS_WIDTH.
  SlotUsageAvailable,     // CurrentUsage;             ///< The enumeration value from MISC_SLOT_USAGE.
  SlotLengthOther,        // SlotLength;               ///< The enumeration value from MISC_SLOT_LENGTH.
  0,    // SlotID;
  {    // SlotCharacteristics1;
    1,  // CharacteristicsUnknown  :1;
    0,  // Provides50Volts         :1;
    0,  // Provides33Volts         :1;
    0,  // SharedSlot              :1;
    0,  // PcCard16Supported       :1;
    0,  // CardBusSupported        :1;
    0,  // ZoomVideoSupported      :1;
    0,  // ModemRingResumeSupported:1;
  },
  {     // SlotCharacteristics2;
    0,  // PmeSignalSupported      :1;
    0,  // HotPlugDevicesSupported :1;
    0,  // SmbusSignalSupported    :1;
    0,  // Reserved                :5;  ///< Set to 0.
  },
  0xFFFF, // SegmentGroupNum;
  0xFF,   // BusNum;
  0xFF,   // DevFuncNum;
};
CHAR8 *mSysSlotInfoType9Strings[] = {
  "SD Card",
  NULL
};


/***********************************************************************
        SMBIOS data definition  TYPE 11  OEM Strings
************************************************************************/

CHAR8 mOemInfoProductUrl[128];

SMBIOS_TABLE_TYPE11 mOemStringsType11 = {
  { EFI_SMBIOS_TYPE_OEM_STRINGS, sizeof (SMBIOS_TABLE_TYPE11), 0 },
  1 // StringCount
};
CHAR8 *mOemStringsType11Strings[] = {
  mOemInfoProductUrl,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE16  Physical Memory ArrayInformation
************************************************************************/
SMBIOS_TABLE_TYPE16 mPhyMemArrayInfoType16 = {
  { EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, sizeof (SMBIOS_TABLE_TYPE16), 0 },
  MemoryArrayLocationSystemBoard, // Location;                       ///< The enumeration value from MEMORY_ARRAY_LOCATION.
  MemoryArrayUseSystemMemory,     // Use;                            ///< The enumeration value from MEMORY_ARRAY_USE.
  MemoryErrorCorrectionUnknown,   // MemoryErrorCorrection;          ///< The enumeration value from MEMORY_ERROR_CORRECTION.
  0x00000000,                     // MaximumCapacity;
  0xFFFE,                         // MemoryErrorInformationHandle;
  1,                              // NumberOfMemoryDevices;
  0x00000000ULL,                  // ExtendedMaximumCapacity;
};
CHAR8 *mPhyMemArrayInfoType16Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE17  Memory Device Information
************************************************************************/
CHAR8 mMemDevInfoVendor[128];

SMBIOS_TABLE_TYPE17 mMemDevInfoType17 = {
  { EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof (SMBIOS_TABLE_TYPE17), 0 },
  0,                    // MemoryArrayHandle; // Should match SMBIOS_TABLE_TYPE16.Handle, initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  0xFFFE,               // MemoryErrorInformationHandle; (not provided)
  0xFFFF,               // TotalWidth; (unknown)
  0xFFFF,               // DataWidth; (unknown)
  0xFFFF,               // Size; // When bit 15 is 0: Size in MB
                        // When bit 15 is 1: Size in KB, and continues in ExtendedSize
                        // initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  MemoryFormFactorChip, // FormFactor;                     ///< The enumeration value from MEMORY_FORM_FACTOR.
  0,                    // DeviceSet;
  1,                    // DeviceLocator String
  0,                    // BankLocator String
  MemoryTypeLpddr4,     // MemoryType;                     ///< The enumeration value from MEMORY_DEVICE_TYPE.
  {                     // TypeDetail;
    0,  // Reserved        :1;
    0,  // Other           :1;
    1,  // Unknown         :1;
    0,  // FastPaged       :1;
    0,  // StaticColumn    :1;
    0,  // PseudoStatic    :1;
    0,  // Rambus          :1;
    0,  // Synchronous     :1;
    0,  // Cmos            :1;
    0,  // Edo             :1;
    0,  // WindowDram      :1;
    0,  // CacheDram       :1;
    0,  // Nonvolatile     :1;
    0,  // Registered      :1;
    0,  // Unbuffered      :1;
    0,  // Reserved1       :1;
  },
  0,                    // Speed; (unknown)
  2,                    // Manufacturer String
  0,                    // SerialNumber String
  0,                    // AssetTag String
  0,                    // PartNumber String
  0,                    // Attributes; (unknown rank)
  0,                    // ExtendedSize; (since Size < 32GB-1)
  0,                    // ConfiguredMemoryClockSpeed; (unknown)
  0,                    // MinimumVoltage; (unknown)
  0,                    // MaximumVoltage; (unknown)
  0,                    // ConfiguredVoltage; (unknown)
  MemoryTechnologyDram, // MemoryTechnology                 ///< The enumeration value from MEMORY_DEVICE_TECHNOLOGY
  {{                    // MemoryOperatingModeCapability
    0,  // Reserved                        :1;
    0,  // Other                           :1;
    0,  // Unknown                         :1;
    1,  // VolatileMemory                  :1;
    0,  // ByteAccessiblePersistentMemory  :1;
    0,  // BlockAccessiblePersistentMemory :1;
    0   // Reserved                        :10;
  }},
  0,                    // FirwareVersion
  0,                    // ModuleManufacturerID (unknown)
  0,                    // ModuleProductID (unknown)
  0,                    // MemorySubsystemControllerManufacturerID (unknown)
  0,                    // MemorySubsystemControllerProductID (unknown)
  0,                    // NonVolatileSize
  0xFFFFFFFFFFFFFFFFULL,// VolatileSize // initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  0,                    // CacheSize
  0,                    // LogicalSize (since MemoryType is not MemoryTypeLogicalNonVolatileDevice)
  0,                    // ExtendedSpeed,
  0                     // ExtendedConfiguredMemorySpeed
};
CHAR8 *mMemDevInfoType17Strings[] = {
  "SDRAM",
  mMemDevInfoVendor,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE19  Memory Array Mapped Address Information
************************************************************************/
SMBIOS_TABLE_TYPE19 mMemArrMapInfoType19 = {
  { EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, sizeof (SMBIOS_TABLE_TYPE19), 0 },
  0x00000000, // StartingAddress;
  0x00000000, // EndingAddress;
  0,          // MemoryArrayHandle; // Should match SMBIOS_TABLE_TYPE16.Handle, initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  1,          // PartitionWidth;
  0,          // ExtendedStartingAddress;  // not used
  0,          // ExtendedEndingAddress;    // not used
};
CHAR8 *mMemArrMapInfoType19Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE32  Boot Information
************************************************************************/
SMBIOS_TABLE_TYPE32 mBootInfoType32 = {
  { EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION, sizeof (SMBIOS_TABLE_TYPE32), 0 },
  { 0, 0, 0, 0, 0, 0 },         // Reserved[6];
  BootInformationStatusNoError  // BootStatus
};

CHAR8 *mBootInfoType32Strings[] = {
  NULL
};


/**

   Create SMBIOS record.

   Converts a fixed SMBIOS structure and an array of pointers to strings into
   an SMBIOS record where the strings are cat'ed on the end of the fixed record
   and terminated via a double NULL and add to SMBIOS table.

   SMBIOS_TABLE_TYPE32 gSmbiosType12 = {
   { EFI_SMBIOS_TYPE_SYSTEM_CONFIGURATION_OPTIONS, sizeof (SMBIOS_TABLE_TYPE12), 0 },
   1 // StringCount
   };

   CHAR8 *gSmbiosType12Strings[] = {
   "Not Found",
   NULL
   };

   ...

   LogSmbiosData (
   (EFI_SMBIOS_TABLE_HEADER*)&gSmbiosType12,
   gSmbiosType12Strings
   );

   @param  Template    Fixed SMBIOS structure, required.
   @param  StringPack  Array of strings to convert to an SMBIOS string pack.
   NULL is OK.
   @param  DataSmbiosHandle  The new SMBIOS record handle.
   NULL is OK.
**/

EFI_STATUS
EFIAPI
LogSmbiosData (
  IN  EFI_SMBIOS_TABLE_HEADER *Template,
  IN  CHAR8                   **StringPack,
  OUT EFI_SMBIOS_HANDLE       *DataSmbiosHandle
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINTN                     Index;
  UINTN                     StringSize;
  UINTN                     Size;
  CHAR8                     *Str;

  //
  // Locate Smbios protocol.
  //
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Calculate the size of the fixed record and optional string pack

  Size = Template->Length;
  if (StringPack == NULL) {
    // At least a double null is required
    Size += 2;
  } else {
    for (Index = 0; StringPack[Index] != NULL; Index++) {
      StringSize = AsciiStrSize (StringPack[Index]);
      Size += StringSize;
    }
    if (StringPack[0] == NULL) {
      // At least a double null is required
      Size += 1;
    }

    // Don't forget the terminating double null
    Size += 1;
  }

  // Copy over Template
  Record = (EFI_SMBIOS_TABLE_HEADER*)AllocateZeroPool (Size);
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, Template, Template->Length);

  // Append string pack
  Str = ((CHAR8*)Record) + Record->Length;

  for (Index = 0; StringPack[Index] != NULL; Index++) {
    StringSize = AsciiStrSize (StringPack[Index]);
    CopyMem (Str, StringPack[Index], StringSize);
    Str += StringSize;
  }

  *Str = 0;
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (
                     Smbios,
                     gImageHandle,
                     &SmbiosHandle,
                     Record
                   );

  if ((Status == EFI_SUCCESS) && (DataSmbiosHandle != NULL)) {
    *DataSmbiosHandle = SmbiosHandle;
  }

  ASSERT_EFI_ERROR (Status);
  FreePool (Record);
  return Status;
}

/***********************************************************************
        SMBIOS data update  TYPE0  BIOS Information
************************************************************************/
VOID
BIOSInfoUpdateSmbiosType0 (
  VOID
  )
{
  INTN   i;
  INTN   State = 0;
  INTN   Value[2];
  INTN   Year = TIME_BUILD_YEAR;
  INTN   Month = TIME_BUILD_MONTH;
  INTN   Day = TIME_BUILD_DAY;

  mBIOSInfoType0.EmbeddedControllerFirmwareMajorRelease = 0;
  mBIOSInfoType0.EmbeddedControllerFirmwareMinorRelease = 0;

  // mBiosVendor and mBiosVersion, which are referenced in mBIOSInfoType0Strings,
  // are left unchanged if the following calls fail.
  UnicodeStrToAsciiStrS ((CHAR16*)PcdGetPtr (PcdFirmwareVendor),
                         mBiosVendor, sizeof (mBiosVendor));
  UnicodeStrToAsciiStrS ((CHAR16*)PcdGetPtr (PcdFirmwareVersionString),
                         mBiosVersion, sizeof (mBiosVersion));
  ASSERT (Year >= 0 && Year <= 9999);
  ASSERT (Month >= 1 && Month <= 12);
  ASSERT (Day >= 1 && Day <= 31);
  AsciiSPrint (mBiosDate, sizeof (mBiosDate), "%02d/%02d/%04d", Month, Day, Year);

  // Look for a "x.y" numeric string anywhere in mBiosVersion and
  // try to parse it to populate the BIOS major and minor.
  for (i = 0; (i < AsciiStrLen (mBiosVersion)) && (State < 4); i++) {
    switch (State) {
    case 0:
      if (!SMB_IS_DIGIT (mBiosVersion[i]))
        break;
      Value[0] = Value[1] = 0;
      State++;
      // Fall through
    case 1:
    case 3:
      if (SMB_IS_DIGIT (mBiosVersion[i])) {
        Value[State / 2] = (Value[State / 2] * 10) + (mBiosVersion[i] - '0');
        if (Value[State / 2] > 255) {
          while (SMB_IS_DIGIT (mBiosVersion[i + 1]))
            i++;
          // Reset our state (we may have something like "Firmware X83737.1 v1.23")
          State = 0;
        }
      } else {
        State++;
      }
      if (State != 2)
        break;
      // Fall through
    case 2:
      if ((mBiosVersion[i] == '.') && (SMB_IS_DIGIT (mBiosVersion[i + 1]))) {
        State++;
      } else {
        State = 0;
      }
      break;
    }
  }
  if ((State == 3) || (State == 4)) {
    mBIOSInfoType0.SystemBiosMajorRelease = (UINT8)Value[0];
    mBIOSInfoType0.SystemBiosMinorRelease = (UINT8)Value[1];
  }

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBIOSInfoType0, mBIOSInfoType0Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE1  System Information
************************************************************************/
VOID
I64ToHexString (
  IN OUT CHAR8* TargetStringSz,
  IN UINT32 TargetStringSize,
  IN UINT64 Value
  )
{
  static CHAR8 ItoH[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
  UINT8 StringInx;
  INT8 NibbleInx;

  ZeroMem ((void*)TargetStringSz, TargetStringSize);

  //
  // Convert each nibble to hex string, starting from
  // the highest non-zero nibble.
  //
  StringInx = 0;
  for (NibbleInx = sizeof (UINT64) * 2; NibbleInx > 0; --NibbleInx) {
    UINT64 NibbleMask = (((UINT64)0xF) << ((NibbleInx - 1) * 4));
    UINT8 Nibble = (UINT8)((Value & NibbleMask) >> ((NibbleInx - 1) * 4));

    ASSERT (Nibble <= 0xF);

    if (StringInx < (TargetStringSize - 1)) {
      TargetStringSz[StringInx++] = ItoH[Nibble];
    } else {
      break;
    }
  }
}

VOID
SysInfoUpdateSmbiosType1 (
  VOID
  )
{
  UINT8 OtpData[16];
  UINT8 SerialLo[8];
  UINT8 SerialHi[8];
  UINT32 BoardRevision = 0;
  UINT64 BoardSerial = 0;
  UINTN Index;

  // Get serial number from OTP
  OtpRead (0x07, sizeof (OtpData), OtpData);
  for (Index = 0; Index < 8; Index++) {
    SerialLo[Index] = OtpData[Index * 2 + 1];
    SerialHi[Index] = OtpData[Index * 2];
  }
  BoardSerial = CalculateCrc32NoComp (0, SerialLo, sizeof SerialLo);
  BoardSerial |= (UINT64)CalculateCrc32NoComp (BoardSerial, SerialHi, sizeof SerialHi) << 32;

  AsciiStrCpyS (mSysInfoProductName, sizeof (mSysInfoProductName), (CHAR8 *) PcdGetPtr(PcdPlatformName));
  AsciiStrCpyS (mSysInfoFamilyName, sizeof (mSysInfoFamilyName), (CHAR8 *) PcdGetPtr(PcdFamilyName));
  AsciiStrCpyS (mSysInfoManufName, sizeof (mSysInfoManufName), (CHAR8 *) PcdGetPtr(PcdPlatformVendorName));
  AsciiSPrint (mSysInfoVersionName, sizeof (mSysInfoVersionName), "%X", BoardRevision);

  I64ToHexString (mSysInfoSKU, sizeof (mSysInfoSKU), BoardRevision);
  I64ToHexString (mSysInfoSerial, sizeof (mSysInfoSerial), BoardSerial);

  DEBUG ((DEBUG_ERROR, "Board Serial Number: %a\n", mSysInfoSerial));

  mSysInfoType1.Uuid.Data1 = BoardRevision;
  mSysInfoType1.Uuid.Data2 = 0x0;
  mSysInfoType1.Uuid.Data3 = 0x0;
  // Swap endianness, so that the serial is more user-friendly as a UUID
  BoardSerial = SwapBytes64 (BoardSerial);
  CopyMem (mSysInfoType1.Uuid.Data4, &BoardSerial, sizeof (BoardSerial));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mSysInfoType1, mSysInfoType1Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE2  Board Information
************************************************************************/
VOID
BoardInfoUpdateSmbiosType2 (
  VOID
  )
{

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBoardInfoType2, mBoardInfoType2Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE3  Enclosure Information
************************************************************************/
VOID
EnclosureInfoUpdateSmbiosType3 (
  VOID
  )
{
  EFI_SMBIOS_HANDLE SmbiosHandle;

  // SMBIOS referenced strings cannot be NULL. If no AssetTag is set, default to a blank space.
  UnicodeStrToAsciiStrS(L" ", mChassisAssetTag, sizeof(mChassisAssetTag));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mEnclosureInfoType3, mEnclosureInfoType3Strings, &SmbiosHandle);

  // Set Type2 ChassisHandle to point to the newly added Type3 handle
  mBoardInfoType2.ChassisHandle = (UINT16) SmbiosHandle;
}


STATIC UINT32
ProcessorGetRate (
  VOID
  )
{
  EFI_STATUS Status;
  SCMI_CLOCK_PROTOCOL *ClockProtocol;
  EFI_GUID ClockProtocolGuid = ARM_SCMI_CLOCK_PROTOCOL_GUID;
  UINT64 Rate;
  UINT32 ClockId = 0;

  // If we can't query SCMI, fallback to reading from CRU registers
  Rate = CruGetCoreClockRate ();

  Status = gBS->LocateProtocol (
                  &ClockProtocolGuid,
                  NULL,
                  (VOID**)&ClockProtocol
                  );
  if (EFI_ERROR (Status)) {
    return (UINT32)Rate;
  }

  ClockProtocol->RateGet (ClockProtocol, ClockId, &Rate);

  DEBUG ((DEBUG_INFO, "SCMI: SMBIOS reported rate %luHz\n", Rate));
  
  return (UINT32)Rate;
}

/***********************************************************************
        SMBIOS data update  TYPE4  Processor Information
************************************************************************/
VOID
ProcessorInfoUpdateSmbiosType4 (
  IN UINTN MaxCpus
  )
{
  UINT32 Rate;
  UINT64 *ProcessorId;

  mProcessorInfoType4.CoreCount = (UINT8)MaxCpus;
  mProcessorInfoType4.CoreCount2 = (UINT8)MaxCpus;
  mProcessorInfoType4.EnabledCoreCount = (UINT8)MaxCpus;
  mProcessorInfoType4.EnabledCoreCount2 = (UINT8)MaxCpus;
  mProcessorInfoType4.ThreadCount = (UINT8)MaxCpus;
  mProcessorInfoType4.ThreadCount2 = (UINT8)MaxCpus;

  Rate = ProcessorGetRate ();
  mProcessorInfoType4.MaxSpeed = Rate / 1000000;
  mProcessorInfoType4.CurrentSpeed = Rate / 1000000;

  AsciiStrCpyS (mCpuName, sizeof (mCpuName), (CHAR8 *) PcdGetPtr(PcdCpuName));

  ProcessorId = (UINT64 *)&(mProcessorInfoType4.ProcessorId);
  *ProcessorId = ArmReadMidr();

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mProcessorInfoType4, mProcessorInfoType4Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE7  Cache Information
************************************************************************/
VOID
CacheInfoUpdateSmbiosType7 (
  VOID
  )
{
  EFI_SMBIOS_HANDLE SmbiosHandle;

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mCacheInfoType7_L1I, mCacheInfoType7Strings_L1I, NULL);

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mCacheInfoType7_L1D, mCacheInfoType7Strings_L1D, &SmbiosHandle);
  // Set Type4 L1CacheHandle to point to the newly added L1 Data Cache
  mProcessorInfoType4.L1CacheHandle = (UINT16) SmbiosHandle;

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mCacheInfoType7_L2, mCacheInfoType7Strings_L2, &SmbiosHandle);
  // Set Type4 L2CacheHandle to point to the newly added L2 Cache
  mProcessorInfoType4.L2CacheHandle = (UINT16) SmbiosHandle;
}

/***********************************************************************
        SMBIOS data update  TYPE9  System Slot Information
************************************************************************/
VOID
SysSlotInfoUpdateSmbiosType9 (
  VOID
  )
{
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mSysSlotInfoType9, mSysSlotInfoType9Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE11  OEM Strings
************************************************************************/
VOID
OemStringsUpdateSmbiosType11 (
  VOID
  )
{
  AsciiStrCpyS (mOemInfoProductUrl, sizeof (mOemInfoProductUrl), (CHAR8 *) PcdGetPtr(PcdProductUrl));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mOemStringsType11, mOemStringsType11Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE16  Physical Memory Array Information
************************************************************************/
VOID
PhyMemArrayInfoUpdateSmbiosType16 (
  VOID
  )
{
  EFI_SMBIOS_HANDLE MemArraySmbiosHandle;

 //
 // Update memory size fields:
 //  - Type 16 MaximumCapacity in KB
 //  - Type 17 size in MB (since bit 15 = 0)
 //  - Type 17 VolatileSize in Bytes
 //

  mMemDevInfoType17.Size = mMemorySize / (1024 * 1024);

  mPhyMemArrayInfoType16.MaximumCapacity = mMemDevInfoType17.Size * 1024; // Size in KB
  mMemDevInfoType17.VolatileSize = MultU64x32 (mMemDevInfoType17.Size, 1024 * 1024);  // Size in Bytes

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mPhyMemArrayInfoType16, mPhyMemArrayInfoType16Strings, &MemArraySmbiosHandle);

  //
  // Update the memory device information and memory array map with the newly added type 16 handle
  //
  mMemDevInfoType17.MemoryArrayHandle = MemArraySmbiosHandle;
  mMemArrMapInfoType19.MemoryArrayHandle = MemArraySmbiosHandle;
}

/***********************************************************************
        SMBIOS data update  TYPE17  Memory Device Information
************************************************************************/
VOID
MemDevInfoUpdateSmbiosType17 (
  VOID
  )
{
  AsciiStrCpyS (mMemDevInfoVendor, sizeof (mMemDevInfoVendor), (CHAR8 *) PcdGetPtr(PcdMemoryVendorName));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mMemDevInfoType17, mMemDevInfoType17Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE19  Memory Array Map Information
************************************************************************/
VOID
MemArrMapInfoUpdateSmbiosType19 (
  VOID
  )
{
  // Note: Type 19 addresses are expressed in KB, not bytes
  mMemArrMapInfoType19.StartingAddress = PcdGet64(PcdSystemMemoryBase) / 1024;

  mMemArrMapInfoType19.EndingAddress = mMemArrMapInfoType19.StartingAddress +
    mMemorySize / 1024 - 1;

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mMemArrMapInfoType19, mMemArrMapInfoType19Strings, NULL);
}


/***********************************************************************
        SMBIOS data update  TYPE32  Boot Information
************************************************************************/
VOID
BootInfoUpdateSmbiosType32 (
  VOID
  )
{
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBootInfoType32, mBootInfoType32Strings, NULL);
}

/***********************************************************************
        Driver Entry
************************************************************************/
EFI_STATUS
EFIAPI
PlatformSmbiosDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  DEBUG ((DEBUG_INFO, "PlatformSmbiosDriverEntryPoint() called\n"));

  mMemorySize = SdramGetMemorySize ();

  BIOSInfoUpdateSmbiosType0 ();

  SysInfoUpdateSmbiosType1 ();

  EnclosureInfoUpdateSmbiosType3 (); // Add Type 3 first to get chassis handle for use in Type 2

  BoardInfoUpdateSmbiosType2 ();

  CacheInfoUpdateSmbiosType7 (); // Add Type 7 first to get Cache handle for use in Type 4

  ProcessorInfoUpdateSmbiosType4 (4);   //One example for creating and updating

  SysSlotInfoUpdateSmbiosType9 ();

  OemStringsUpdateSmbiosType11 ();

  PhyMemArrayInfoUpdateSmbiosType16 ();

  MemDevInfoUpdateSmbiosType17 ();

  MemArrMapInfoUpdateSmbiosType19 ();

  BootInfoUpdateSmbiosType32 ();

  DEBUG ((DEBUG_INFO, "PlatformSmbiosDriverEntryPoint() returning\n"));

  return EFI_SUCCESS;
}
