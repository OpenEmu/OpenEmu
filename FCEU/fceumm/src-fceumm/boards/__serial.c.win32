
#include "__serial.h"

HANDLE SerialPort = NULL;  // Handle of SerialPort itself.

BOOL SerialOpen(int port, int baud)
{
  HANDLE Comport;
  DCB myDCB;
  COMMTIMEOUTS CTout;
  char str[100];

  if (port > 9)
    sprintf(str, "\\\\.\\COM%d", port);
  else
    sprintf(str, "COM%d", port);

  // Open the serial port
  if ((Comport = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
    return FALSE;

  // Configure Serial port (Setup Comm)

  // Buffer sizes
  if (!SetupComm(Comport, 128, 128))
    return FALSE;

  // Setup DCB using current values
  if (!GetCommState(Comport, &myDCB))
    return FALSE;

  myDCB.fInX = FALSE;     // Turn off xon/xoff handler
  myDCB.fOutX = FALSE;
  myDCB.fOutxDsrFlow = FALSE;
  myDCB.fOutxCtsFlow = FALSE;    // no hardware flow control.
  myDCB.BaudRate = baud;
  myDCB.DCBlength = sizeof(DCB);
  myDCB.fBinary = 1;
  myDCB.fParity = 0;
  myDCB.fDtrControl = DTR_CONTROL_DISABLE;
  myDCB.fDsrSensitivity = 0;
  myDCB.fTXContinueOnXoff = 1;
  myDCB.fNull = 0;
  myDCB.fRtsControl = RTS_CONTROL_DISABLE;
  myDCB.fDummy2 = 0;
  myDCB.wReserved = 0;
  myDCB.Parity = NOPARITY;
  myDCB.StopBits = ONESTOPBIT;
  myDCB.wReserved1 = 0;
  myDCB.ByteSize = 8;

  if (!SetCommState(Comport, &myDCB))
	return FALSE;

  // Set timeouts
  CTout.ReadIntervalTimeout = 0xffffffff;
  CTout.ReadTotalTimeoutMultiplier = 0;
  CTout.ReadTotalTimeoutConstant = 0;
  CTout.WriteTotalTimeoutMultiplier = 0;
  CTout.WriteTotalTimeoutConstant = 5000;         // don't hang if CTS is locked, for example

  SetCommTimeouts(Comport, &CTout);
  EscapeCommFunction(Comport, SETDTR);
  PurgeComm(Comport, PURGE_TXCLEAR | PURGE_RXCLEAR);

  SerialPort = Comport;

  return TRUE;
}

void SerialClose(void)
{
  if (SerialPort == NULL) return;

  PurgeComm(SerialPort, PURGE_TXCLEAR | PURGE_RXCLEAR);
  CloseHandle(SerialPort);

  SerialPort = NULL;
}

BOOL SerialSendChar(int c)
{
  DWORD cr;
  if(WriteFile(SerialPort, &c, 1, (LPDWORD)&cr, NULL) && cr)
    return TRUE;
  else
    return FALSE;
}

int SerialIsOpen(void)
{
  return (SerialPort != NULL);
}

int SerialGetChar(void)
{
  uint8 ch;
  DWORD cr;
  if (SerialPort != NULL) {
    if (ReadFile(SerialPort, &ch, 1, (LPDWORD)&cr, NULL) && cr)
      return (int)ch;
  }
  return EOF;
}

void SendCmd(uint8 *cmd, int size)
{
   int i;
   for(i=0; i<size; i++) {
     SerialSendChar(cmd[i]);
   }
}

int ReadResp(uint8 *resp, int size)
{
   int i = 0, sum = 0, data;
   while(i < size) {
	   while ((data = SerialGetChar()) == EOF) {}
       resp[i] = data & 0xff;
       sum += (data & 0xff);
       i++;
   }
   return sum;
}
