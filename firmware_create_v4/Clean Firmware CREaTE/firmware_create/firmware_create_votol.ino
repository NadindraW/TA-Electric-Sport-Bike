/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 31/07/23
 *
 * VOTOL is one of electric motor controller unit (E-MCU) brand. ----------------------------
 * Thus, make sure that our EV use VOTOL to work properly. 
 * Data including Voltage, current, speed, temperature, mode and state is available to access.
 */

#define E_BRAKE_ON            0     // 0x00000001
#define OVER_CURRENT          1     // 0x00000002
#define UNDER_VOLTAGE         2     // 0x00000004
#define HALL_ERROR            3     // 0x00000008 
#define OVER_VOLTAGE          4     // 0x00000010
#define MCU_ERROR             5     // 0x00000020
#define MOTOR_BLOCK           6     // 0x00000040
#define FOOTPLATE_ERR         7     // 0x00000080
#define SPEED_CONTROL         8     // 0x00000100
#define WRITING_EEPROM        9     // 0x00000200
#define START_UP_FAILURE      11    // 0x00000800 
#define OVERHEAT              12    // 0x00001000
#define OVER_CURRENT1         13    // 0x00002000
#define ACCELERATE_PADAL_ERR  14    // 0x00004000
#define ICS1_ERR              15    // 0x00008000
#define ICS2_ERR              16    // 0x00010000
#define BREAK_ERR             17    // 0x00020000
#define HAL_SEL_ERROR         18    // 0x00040000
#define MOFSET_DRIVER_FAULT   19    // 0x00080000
#define MOFSET_HIGH_SHORT     20    // 0x00100000
#define PHASE_OPEN            21    // 0x00200000
#define PHASE_SHORT           22    // 0x00400000
#define MCU_CHIP_ERROR        23    // 0x00800000
#define PRE_CHARGE_ERROR      24    // 0x01000000
#define OVERHEAT1             27    // 0x08000000
#define SOC_ZERO_ERROR        31    // 0x80000000

bool head = false;
bool tail = false;
bool headCommand = false;                     // Indentify head and tails of VOTOL Protocol
uint8_t headerVotol[] = { 192, 20, 13, 89};   // It's used for authentication
uint16_t iData = 0;
uint16_t checksum = 0;
uint32_t mVotolFaultCode;
  
void readVotol()
{
    while (Serial2.available() > 0)
    {
        inDataVotol[iData] = Serial2.read();
        if (iData >= 2 && inDataVotol[iData] == headerVotol[2])
        {
            if (inDataVotol[iData - 1] == headerVotol[1] && inDataVotol[iData - 2] == headerVotol[0]) 
            {
                head = true;
                iData = 2;
            }
        }
        if (iData >= 2 && inDataVotol[iData] == cmd_votol[2])
        {
            if (inDataVotol[iData - 1] == cmd_votol[1] && inDataVotol[iData - 2] == cmd_votol[0]) 
            {
                head = true;
                iData = 2;
            }
        }
        iData++;

        if (iData >= 24)
        {
            if (inDataVotol[iData - 1] == (uint8_t)13) tail = true;
            if (head == true && tail == true)
            {
                for (uint8_t i = 0; i < 22; i++)
                {
                    checksum = checksum ^ inDataVotol[i];
                }
                parseVotol();
            }
            tail = false;      
            head = false;
            headCommand = false;
            iData = 0;      
        }        
    }
}

void parseVotol()
{
    mVotolVoltage = (inDataVotol[5] << 8) ^ inDataVotol[6];    // exclude scale 0.1 
    mVotolCurrent = (inDataVotol[7] << 8) ^ inDataVotol[8];    // exclude scale 0.1
    mVotolRPM     = (inDataVotol[14] << 8) ^ inDataVotol[15];
    mVotolConTemp = (inDataVotol[16]) - 50;                    // include offset 50
    mVotolExtTemp = (inDataVotol[17]) - 50;                    // include offset 50
    mVotolMode    = bitRead(inDataVotol[20], 1)*2 + bitRead(inDataVotol[20], 0) + 1;
    mVotolFaultCode = (inDataVotol[10] << 24) ^ (inDataVotol[11] << 16) ^ (inDataVotol[12] << 8) ^ inDataVotol[13];

}

void faultVotol(){
    for (int i=0; i<=31; i++)
    {
        if ((mVotolFaultCode & (1 << i)) != 0) 
        {
            printf("failure: ");
            printVotolFailure(i);
            printf("\n");
            printf("fix the failure: ")
            printVotolSuggestion(i);
            printf("\n");
        }
        // else printf("normal\n");
    }
}


void printVotolFailure(int i)
{
    switch(i)
	  {
      case E_BRAKE_ON:
        printf("brake");
        break;
      case OVER_CURRENT:
        printf("hardware overcurrent");
        break;
      case UNDER_VOLTAGE:
        printf("undervoltage");
        break;
      case HALL_ERROR:
        printf("hall failure");
        break;
      case OVER_VOLTAGE:
        printf("over voltage");
        break;
      case MCU_ERROR:
        printf("controller failure");
        break;
      case MOTOR_BLOCK:
        printf("lock motor failure");
        break;
      case FOOTPLATE_ERR:
        printf("trhottle failure");
        break;
      case SPEED_CONTROL:
        printf("flying car");
        break;
      case WRITING_EEPROM:
        printf("EEPROM WRITE");
        break;
      case START_UP_FAILURE:
        printf("QC failure");
        break;
      case OVERHEAT:
        printf("controller overheating");
        break;
      case OVER_CURRENT1:
        printf("software overcurrent");
        break;
      case ACCELERATE_PADAL_ERR:
        printf("throttle failure");
        break;
      case ICS1_ERR:
        printf("current sensor failure");
        break;
      case ICS2_ERR:
        printf("current sensor failure");
        break;
      case BREAK_ERR:
        printf("brake failure");
        break;
      case HAL_SEL_ERROR:
        printf("hall");
        break;
      case MOFSET_DRIVER_FAULT:
        printf("drive failure");
        break;
      case MOFSET_HIGH_SHORT:
        printf("MOS tube high side short circuit");
        break;
      case PHASE_OPEN:
        printf("lack of phase");
        break;
      case PHASE_SHORT:
        printf("phase short circuit controller failure");
        break;
      case MCU_CHIP_ERROR:
        printf("controller failure");
        break;
      case PRE_CHARGE_ERROR:
        printf("precharge failure");
        break;
      case OVERHEAT1:
        printf("motor overheating");
        break;
      case SOC_ZERO_ERROR:
        printf("SOC is zero");
        break;
      default :
        break;
	  }
}

void printVotolSuggestion(int i)
{
    switch(i)
	  {
      case E_BRAKE_ON:
        printf("Release brake signal, high level is effective, release high level, low level is effective, ground");
        break;
      case OVER_CURRENT:
        printf("No suggestion available");
        break;
      case UNDER_VOLTAGE:
        printf("Check the battery voltage / reduce the undervoltage value");
        break;
      case HALL_ERROR:
        printf("Check whether the Hall wire is loose/the sequence of the phase wires is correct/whether the Hall is faulty");
        break;
      case OVER_VOLTAGE:
        printf("Check the battery voltage/adjust the overvoltage value");
        break;
      case MCU_ERROR:
        printf("Restart controller");
        break;
      case MOTOR_BLOCK:
        printf("Motor stalled, check for overload");
        break;
      case FOOTPLATE_ERR:
        printf("Check whether the wiring sequence of the handle/operating voltage range matches/turn the handle back to zero and start again");
        break;
      case SPEED_CONTROL:
        printf("Verify that the phase shift angle match is accurate");
        break;
      case WRITING_EEPROM:
        printf("Program upload error, re-upload the program");
        break;
      case START_UP_FAILURE:
        printf("No suggestion available");
        break;
      case OVERHEAT:
        printf("The power does not match, replace a controller with a higher power or a motor with a smaller power");
        break;
      case OVER_CURRENT1:
        printf("No suggestion available");
        break;
      case ACCELERATE_PADAL_ERR:
        printf("Check whether the wiring sequence of the handle/operating voltage range matches/turn the handle back to zero and start again");
        break;
      case ICS1_ERR:
        printf("No suggestion available");
        break;
      case ICS2_ERR:
        printf("No suggestion available");
        break;
      case BREAK_ERR:
        printf("Release brake signal, high level is effective, release high level, low level is effective, ground");
        break;
      case HAL_SEL_ERROR:
        printf("Check whether the Hall wire is loose/the sequence of the phase wires is correct/whether the Hall is faulty");
        break;
      case MOFSET_DRIVER_FAULT:
        printf("Check the phase sequence of the motor/whether the motor is faulty");
        break;
      case MOFSET_HIGH_SHORT:
        printf("No suggestion available");
        break;
      case PHASE_OPEN:
        printf("Check the phase sequence of the motor line and whether there is a virtual connection");
        break;
      case PHASE_SHORT:
        printf("Check the phase sequence of the motor line and whether there is a short circuit");
        break;
      case MCU_CHIP_ERROR:
        printf("No suggestion available");
        break;
      case PRE_CHARGE_ERROR:
        printf("No suggestion available");
        break;
      case OVERHEAT1:
        printf("The power does not match, replace a motor with a higher power or a controller with a lower power");
        break;
      case SOC_ZERO_ERROR:
        printf("Detect whether the signal line sequence and protocol match");
        break;
      default :
        break;
	  }
}