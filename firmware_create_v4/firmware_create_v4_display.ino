/*
 * Developed by Febby Ronaldo (https://github.com/FeroVbee)
 * Other Contributors: Thanks to Nobby & Nico
 * Funded by PRIET and PT. VKTR Teknologi Mobilitas
 * LAST REVISION: 29/04/23
 */

uint8_t send_dash[9];                                         // It's used for Dash App needs
uint8_t cmd_votol[24] = { 201, 20, 2, 83, 72, 79, 87, 0,      // Command to connect VOTOL
                          0, 0, 0, 0, 170, 17, 226, 0, 
                          37, 170, 5, 176, 0, 63, 128, 13};

void uart_send_dashboard_UI(){
    Serial2.write(cmd_votol,24);   
    // read_votol();   
    delay(10);
    read_votol();   // Verification
    send_dash[0] = show_speed();
    send_dash[1] = show_throt();
    send_dash[2] = show_miles_lsb();
    send_dash[3] = show_miles_msb();
    send_dash[4] = show_battr() ^ (show_night() << 7);
    send_dash[5] = show_enginetemp() ^ (show_seinx() << 6);
    send_dash[6] = show_battrtemp() ^ (show_horns() << 6) ^ (show_beams() << 7);
    send_dash[7] = show_alert() ^ (show_state() << 3) ^ (show_modes() << 5) ^ (show_maps() << 7);
    send_dash[8] = '\n';
    // SerialBT.write(send_dash,9);       
    delay(150);
}

/* B0 SPEED PROTOCOL
 * Start Bit Pos. : 0          Scale   : 1      Inter value    : 0-255   
 * Data Length    : 8 bit      Offset  : 0      Unit    : Km/h 
 */
uint8_t show_speed(){
    // k(km/h) = d(cm) × r(RPM) × 0.001885 
    uint8_t val_speed = rpm_vtl / 46;
    Serial.print("obj_spd.val=");
    Serial.print(val_speed);
    Serial.print("\xFF\xFF\xFF");
    return val_speed;
}

/* B1 THROTTLE PROTOCOL
 * Start Bit Pos. : 8          Scale   : 1      Inter value    : 0-255
 * Data Length    : 8 bit      Offset  : 0      Unit    : null 
 */
uint8_t show_throt() {
    return 0;
}

/* B2-B3 TOTAL DISCTANCE PROTOCOL
 * Start Bit Pos. : 16         Scale   : 0.5    Inter value    : 0-65535  
 * Data Length    : 16 bit     Offset  : 0      Unit    : Km 
 */  
uint8_t show_miles_lsb() {
    uint16_t val_miles = 300000 * 0.5;
    return val_miles;
}
uint8_t show_miles_msb() {
    uint16_t val_miles = 300000 * 0.5;
    return (val_miles >> 8);
}

/* B4(L) BATTERY PROTOCOL
 * Start Bit Pos. : 32         Scale   : 1      Inter value    : 0-100
 * Data Length    : 7 bit      Offset  : 0      Unit    : %        
 */
uint8_t show_battr() {  // raw data in 630 - 830 to translate in 0 - 100
    // uint16_t val_battr = ((voltage_vtl > 620) ? voltage_vtl - 620 : 0);  // using power supply
    uint16_t val_battr = ((voltage_vtl > 630) ? (voltage_vtl-630)/2 : 0);  // using EV battery
    if (val_battr > 100) val_battr = 100;
    Serial.print("obj_bat.val=");
    Serial.print(val_battr);
    Serial.print("\xFF\xFF\xFF");

    if(val_battr<20){
        Serial.print("vis obj_lowbat,1"); 
        Serial.print("\xFF\xFF\xFF");
        Serial.print("vis obj_ntflowdark,1"); 
        Serial.print("\xFF\xFF\xFF");
    }
    else {
        Serial.print("vis obj_lowbat,0"); 
        Serial.print("\xFF\xFF\xFF");
        Serial.print("vis obj_ntflowdark,0"); 
        Serial.print("\xFF\xFF\xFF");
    }
    return val_battr;
}

/* B4(H) NIGHT MODE PROTOCOL
 * Start Bit Pos. : 39         Scale   : 1      Inter value    : 0-1
 * Data Length    : 1 bit      Offset  : 0      Unit    : null       
 */
uint8_t show_night() {
    //   if (digitalRead(NIGHT_PIN) == HIGH) return 1;       // 0x01 - 0b00000001 (NIGHT)
    //   else return 0;                                      // 0x00 - 0b00000000 (DAY)
    return 1;
}

/* B5(L) ENGINE TEMPERATURE PROTOCOL
 * Start Bit Pos. : 40         Scale   : 1      Inter value    : 0-63
 * Data Length    : 6 bit      Offset  : -20    Unit    : °C  
 */
uint8_t show_enginetemp() {
    return contemp_vtl;
}

/* B5(H) SEIN PROTOCOL
 * Start Bit Pos. : 46         Scale   : 1      Inter value    : 0-3 
 * Data Length    : 2 bit      Offset  : 0      Unit    : null 
 */
uint8_t show_seinx() {
    uint8_t valR = analogRead(SEINR_PIN);
    uint8_t valL = analogRead(SEINL_PIN);

    // if (val_sein == 1){
    //     Serial.print("sein.val=1");
    //     Serial.print("\xFF\xFF\xFF");
    //     Serial.println("Kanan");
    //     return 1;  // 0x01 - 0b00000001 (RIGHT) 
    // }
    // else if (val_sein == 2){
    //     Serial.print("sein.val=2");
    //     Serial.print("\xFF\xFF\xFF"); 
    //     Serial.println("Kiri");
    //     return 2;  // 0x02 - 0b00000010 (LEFT)
    // }
    // else 
    if ((valL == 0) && (valR > 0)){
        Serial.print("sein.val=1");
        Serial.print("\xFF\xFF\xFF");
        return 1;  // 0x01 - 0b00000001 (RIGHT) 
    }
    else if ((valR == 0) && (valL > 0)){
        Serial.print("sein.val=2");
        Serial.print("\xFF\xFF\xFF"); 
        return 2;  // 0x02 - 0b00000010 (LEFT)
    }
    else if((valL == 0) && (valR == 0)) {   //val_sein ==  0 || 
        Serial.print("sein.val=0");
        Serial.print("\xFF\xFF\xFF"); 
        return 0;  
    }
     
    if ((valR > 0) && (valL > 0)){
        Serial.print("sein.val=3");
        Serial.print("\xFF\xFF\xFF"); 
        return 3;  // 0x03 - 0b00000011 (HAZARD)
    }
}

/* B6(L1) BATTERY TEMPERATURE PROTOCOL
 * Start Bit Pos. : 48         Scale   : 1      Inter value    : 0-63
 * Data Length    : 6 bit      Offset  : -20    Unit    : °C      
 */
uint8_t show_battrtemp() {
    return 0;
}

/* B6(L2) HORN PROTOCOL
 * Start Bit Pos. : 54         Scale   : 1      Inter value    : 0-1 
 * Data Length    : 1 bit      Offset  : 0      Unit    : null    
 */
uint8_t show_horns() {
  //   if (digitalRead(HORNS_PIN) == HIGH) return 1;  // 0x01 - 0b00000001 (ON)
  //   else return 0;                                 // 0x00 - 0b00000000 (OFF)
    return 0;
}

/* B6(H) BEAM PROTOCOL
 * Start Bit Pos. : 55         Scale   : 1      Inter value    : 0-1  
 * Data Length    : 1 bit      Offset  : 0      Unit    : null  
 */
uint8_t show_beams() {
    if (digitalRead(BEAMS_PIN) > 0) {
        Serial.print("vis obj_beam,1");  // 0x00 - 0b00000000 (ON)
        Serial.print("\xFF\xFF\xFF");
        return 1;
    }
    else{
        Serial.print("vis obj_beam,0");  // 0x01 - 0b00000001 (OFF)
        Serial.print("\xFF\xFF\xFF");
        return 0;
    }
}

/* B7(L1) ALERTS PROTOCOL 
 * Start Bit Pos. : 56         Scale   : 1      Inter value    : 0-8 
 * Data Length    : 3 bit      Offset  : 0      Unit    : null 
 */
uint8_t show_alert() {
    return 0; // NEXT PLAN
}

/* B7(L2) STATE PROTOCOL
 * Start Bit Pos. : 59         Scale   : 1      Inter value    : 0-3
 * Data Length    : 2 bit      Offset  : 0      Unit    : null   
 */
uint8_t show_state() {
    Serial.print("obj_state.txt=");
    if (rpm_vtl > 100) { 
        val_state = 2;  Serial.print("\"D\""); }          // 0x02 - 0b00000010 (DRIVE)   
    else if (bitRead(inData_votol[20], 3) == 1){ 
        val_state = 3; Serial.print("\"P\""); }           // 0x03 - 0b00000011 (PARKING)
    else { val_state = 1; Serial.print("\"N\""); }        // 0x01 - 0b00000001 (NORMAL) 
    Serial.print("\xFF\xFF\xFF");
    return val_state;
}

/* B7(H1) MODE PROTOCOL
 * Start Bit Pos. : 61         Scale   : 1      Inter value    : 0-3  
 * Data Length    : 2 bit      Offset  : 0      Unit    : null   
 */
uint8_t show_modes() {
    Serial.print("obj_mode.txt=");
    if (mode_vtl == 1) Serial.print("\"ECONOMIC\"");      // 0x01 - 0b00000001 (ECON)
    else if (mode_vtl == 2) Serial.print("\"COMFORT\"");  // 0x02 - 0b00000010 (COMF)
    else if (mode_vtl == 3) Serial.print("\"SPORT\"");    // 0x03 - 0b00000011 (SPRT)
    Serial.print("\xFF\xFF\xFF");
    return mode_vtl;
}

/* B7(H2) MAPS SWITCH PROTOCOL
 * Start Bit Pos. : 63         Scale   : 1      Inter value    : 0-1    
 * Data Length    : 1 bit      Offset  : 0      Unit    : null
 */
uint8_t show_maps() {
    //   if (digitalRead(MAPSX_PIN) == HIGH) return 1;    // 0x01 - 0b00000001 (MAPS)
    //   else return 0;                                   // 0x00 - 0b00000000 (SPEEDO)
    return 0;
}
