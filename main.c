#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/*
    Documentations for Windows.h (Windbase.h)
    https://learn.microsoft.com/en-us/windows/win32/api/winbase/
*/



HANDLE IOhandle; /*HANDLE -> void* (function pointer)*/
COMSTAT status; 
DWORD errors;



bool _openPort(char *port,
                uint64_t BAUD_RATE,
                uint8_t BYTE_SIZE,
                uint8_t PARITY
                ){


    IOhandle = CreateFileA((const char *)port,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    if(IOhandle == INVALID_HANDLE_VALUE){
        printf("Could not open COM port : %s\n",port);
        return false;
    }

    DCB serial_parameters = { 0 };
    
    if(!GetCommState(IOhandle,&serial_parameters)){
        printf("Failed to read current serial parameters\n");
        return false;
    }
    
    serial_parameters.BaudRate = BAUD_RATE;
    serial_parameters.ByteSize = BYTE_SIZE;
    serial_parameters.StopBits = ONESTOPBIT;
    serial_parameters.Parity = NOPARITY;
    serial_parameters.fDtrControl = DTR_CONTROL_ENABLE;

    if(!SetCommState(IOhandle,&serial_parameters)){
        printf("Failed to set serial port parameters\n");
        return false;
    }
    PurgeComm(IOhandle,PURGE_RXCLEAR | PURGE_TXCLEAR);

    return true;
}


bool is_valid(char ch){
    if (ch >= 32 && ch <= 126)return true;
    return false;
}


//timeout in seconds

char *_readSerialPort(uint32_t timeout){
    DWORD bytes;
    char inputChar[10];
    char *rcv_input; strcpy(rcv_input,"");

    bool start = false;

    ClearCommError(IOhandle,&errors,&status);

    uint64_t start_time = time(NULL);
    while(time(NULL) - start_time < timeout){
        if(status.cbInQue > 0){

            if (ReadFile(IOhandle,inputChar,1,&bytes,NULL)){
               
                if((inputChar[0] >= 32 && inputChar[0] <= 126) || start){
                    start = true;

                    if(inputChar[0] == '}'){
                        return rcv_input;
                    }
                    if(inputChar[0] != '{'){
                        strcat(rcv_input,inputChar);
                    }
                }
            }
            else{
                return "Failed to read data\n";
            }
        }
    }
 
    return rcv_input;
}




int main()
{
    char port[] = "\\\\.\\COM3";
    bool ifConnected = _openPort(port,CBR_115200,8,0);
    printf("Connection State : %s\n",(ifConnected) ? "TRUE":"FALSE");

    if(ifConnected){
        printf("INSIDE FOR LOOP 1\n___________________\n");
        for(int i = 0;i < 5;i++){
            char *serial_read_data = _readSerialPort(3);
            printf("Recieved : %s\n",serial_read_data);
        }
    }

    return 0;
}