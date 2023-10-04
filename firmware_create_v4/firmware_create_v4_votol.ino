/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 29/04/23
 */
 
bool head = false, tail = false, head_cmd = false;            // Indentify head and tails of VOTOL Protocol
uint8_t header_votol[] = { 192, 20, 13, 89};                  // It's used for authentication
uint16_t n_data = 0, xor_value = 0;
/* VOTOL is one of electric motor controller unit (E-MCU) brand. ----------------------------
 * Thus, make sure that our EV use VOTOL to work properly. 
 * Data including Voltage, current, speed, temperature, mode and state is available to access.
 */

void read_votol(){
    while (Serial2.available() > 0){
      inData_votol[n_data] = Serial2.read();
      if (n_data >= 2 && inData_votol[n_data] == header_votol[2]){
        if (inData_votol[n_data - 1] == header_votol[1] && inData_votol[n_data - 2] == header_votol[0]) {
          head = true;
          n_data = 2;
        }
      }
      if (n_data >= 2 && inData_votol[n_data] == cmd_votol[2]){
        if (inData_votol[n_data - 1] == cmd_votol[1] && inData_votol[n_data - 2] == cmd_votol[0]) {
          head = true;
          n_data = 2;
        }
      }
      n_data++;

      if (n_data >= 24){
        if (inData_votol[n_data - 1] == (uint8_t)13) tail = true;
        if (head == true && tail == true){
          for (uint8_t i = 0; i < 22; i++){
            xor_value = xor_value ^ inData_votol[i];
          }
          parse_votol();
        }
        tail = false;      
        head = false;
        head_cmd = false;
        
        n_data = 0;      
      }        
    }
}

void parse_votol(){
    voltage_vtl = (inData_votol[5] << 8) ^ inData_votol[6];   // exclude scale 0.1 
    ampere_vtl  = (inData_votol[7] << 8) ^ inData_votol[8];   // exclude scale 0.1
    rpm_vtl     = (inData_votol[14] << 8) ^ inData_votol[15];
    contemp_vtl = (inData_votol[16]) - 50;                    // include offset 50
    exttemp_vtl = (inData_votol[17]) - 50;                    // include offset 50
    mode_vtl    = bitRead(inData_votol[20], 1)*2 + bitRead(inData_votol[20], 0) + 1;
}
