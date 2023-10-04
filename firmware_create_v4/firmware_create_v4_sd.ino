
/* ------------------------------------ SD MODULE FUNCTION ----------------------------------------- 
 * -------------------------------------------------------------------------------------------------
 */
void writeFile(fs::FS &fs, const char * path, const char * message){
    // Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        // Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        // Serial.println("File written");
    } else {
        // Serial.println("Write failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    // Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        // Serial.println("File renamed");
    } else {
        // Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    // Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        // Serial.println("File deleted");
    } else {
        // Serial.println("Delete failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    // Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        // Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        // Serial.println("Message appended");
    } else {
        // Serial.println("Append failed");
    }
    file.close();
}

// Back up ---
// while (Serial.available() == 0) {}                    // wait for data available
// String teststr = "/" + Serial.readString() + ".txt";  // read until timeout - get string realtime datetime
// teststr.trim();                                       // remove any \r \n whitespace at the end of the String
// txtname = teststr.c_str();
