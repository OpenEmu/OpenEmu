-------------------------------------------------------------------------------
--       Tables - Assumes connection to ka5200 database established          --
-------------------------------------------------------------------------------
BEGIN;
CREATE TABLE MachineTypes (
    MachineID                       INTEGER PRIMARY KEY,
    Machine                         VARCHAR(50) NOT NULL
);

CREATE TABLE VideoSystemTypes (
    VideoSystemID                   INTEGER PRIMARY KEY,
    VideoSystem                     VARCHAR(50) NOT NULL
);

CREATE TABLE MediaTypes (
    MediaTypeID                     INTEGER PRIMARY KEY,
    MediaType                       VARCHAR(50) NOT NULL
);

CREATE TABLE TosecTypes (
    TosecTypeID                     INTEGER PRIMARY KEY,
    TosecType                       VARCHAR(50) NOT NULL,
    Version                         VARCHAR(50) NOT NULL
);

CREATE TABLE ControllerTypes (
    ControllerID                    INTEGER PRIMARY KEY,
    Controller                      VARCHAR(50) NOT NULL
);

CREATE TABLE KeyboardCodes (
    Keycode                         INTEGER NOT NULL,
    KeyName                         VARCHAR(50) UNIQUE NOT NULL
);

CREATE TABLE KeypadCodes (
    Keycode                         INTEGER NOT NULL,
    KeyName                         VARCHAR(50) UNIQUE NOT NULL
);

CREATE TABLE Config (
    Name                            VARCHAR(50) UNIQUE NOT NULL,
    Description                     VARCHAR,
    RomImageFile                    VARCHAR,
    BiosFile5200                    VARCHAR,
    BiosFile800                     VARCHAR,
    BiosFileXL                      VARCHAR,
    BasicFile                       VARCHAR,
    FontFile                        VARCHAR,
    State                           VARCHAR,
    DefaultInputProfile             VARCHAR,
    Default800InputProfile          VARCHAR,
    DefaultVideoProfile             VARCHAR,
    DefaultSoundProfile             VARCHAR,
    UIKeysProfile                   VARCHAR,
    MachineID                       INTEGER NOT NULL REFERENCES MachineTypes(MachineID),
    VideoSystemID                   INTEGER NOT NULL REFERENCES VideoSystemTypes(VideoSystemID),
    Throttle                        INTEGER NOT NULL,
    RamSize5200                     INTEGER NOT NULL,
    RamSize800                      INTEGER NOT NULL,
    RamSizeXL                       INTEGER NOT NULL,
    SIOPatch                        INTEGER NOT NULL,
    BasicEnable                     INTEGER NOT NULL,
    StartWithGUI                    INTEGER NOT NULL,
    StartGUIWithLauncher            INTEGER NOT NULL,
    LauncherShowAll                 INTEGER NOT NULL,
    LauncherTabbedView              INTEGER NOT NULL,
    LauncherGroupView               INTEGER NOT NULL,
    ExitStatus                      INTEGER NOT NULL,
    FirstUse                        INTEGER NOT NULL
);

CREATE TABLE InputDevices (
    DeviceID                        INTEGER PRIMARY KEY,
    Device                          VARCHAR(50) NOT NULL
);

CREATE TABLE InputKeys (
    KeyID                           INTEGER UNIQUE NOT NULL,
    Key                             VARCHAR(50) NOT NULL
);

CREATE TABLE InputParts (
    PartID                          INTEGER PRIMARY KEY,
    Part                            VARCHAR(50) NOT NULL
);

CREATE TABLE InputDirection (
    DirectionID                     INTEGER PRIMARY KEY,
    Direction                       VARCHAR(50) NOT NULL
);

CREATE TABLE InputList (
    InputID                         INTEGER PRIMARY KEY AUTOINCREMENT,
    Device                          INTEGER NOT NULL REFERENCES InputDevices(DeviceID),
    DeviceNum                       INTEGER NOT NULL,
    Part                            INTEGER NOT NULL REFERENCES InputParts(PartID),
    PartNum                         INTEGER NOT NULL,
    Direction                       INTEGER NOT NULL REFERENCES InputDirection(DirectionID)
);

CREATE TABLE UIKeys (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    Description                     VARCHAR,
    Exit                            INTEGER NOT NULL REFERENCES InputList(InputID),
    Back                            INTEGER NOT NULL REFERENCES InputList(InputID),
    Fullscreen                      INTEGER NOT NULL REFERENCES InputList(InputID),
    Screenshot                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState1                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState2                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState3                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState4                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState5                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState6                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState7                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState8                      INTEGER NOT NULL REFERENCES InputList(InputID),
    LoadState9                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState1                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState2                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState3                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState4                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState5                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState6                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState7                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState8                      INTEGER NOT NULL REFERENCES InputList(InputID),
    SaveState9                      INTEGER NOT NULL REFERENCES InputList(InputID)
);

CREATE TABLE Keyboard (
    KeyboardID                      INTEGER PRIMARY KEY AUTOINCREMENT,
    KEY_L                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_J                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_SEMI                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_K                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_PLUS                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_STAR                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_O                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_P                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_U                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_RETURN                      INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_I                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_MINUS                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_EQUALS                      INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_V                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_HELP                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_C                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_B                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_X                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_Z                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_4                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_3                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_6                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_ESC                         INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_5                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_2                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_1                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_COMMA                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_SPACE                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_PERIOD                      INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_N                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_M                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_SLASH                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_FUJI                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_R                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_E                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_Y                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_TAB                         INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_T                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_W                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_Q                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_9                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_0                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_7                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_BKSP                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_8                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_LESST                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_MORET                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_F                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_H                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_D                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_CAPS                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_G                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_S                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_A                           INTEGER NOT NULL REFERENCES InputList(InputID)
);

CREATE TABLE Keypad (
    KeypadID                        INTEGER PRIMARY KEY AUTOINCREMENT,
    KEY_START                       INTEGER REFERENCES InputList(InputID),
    KEY_PAUSE                       INTEGER REFERENCES InputList(InputID),
    KEY_RESET                       INTEGER REFERENCES InputList(InputID),
    KEY_STAR                        INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_POUND                       INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_0                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_1                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_2                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_3                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_4                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_5                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_6                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_7                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_8                           INTEGER NOT NULL REFERENCES InputList(InputID),
    KEY_9                           INTEGER NOT NULL REFERENCES InputList(InputID)
);

CREATE TABLE Player (
    PlayerID                        INTEGER PRIMARY KEY AUTOINCREMENT,
    ControllerTypeID                INTEGER NOT NULL REFERENCES ControllerTypes(ControllerID),
    DeadZone                        INTEGER NOT NULL,
    Sensitivity                     INTEGER NOT NULL,
    SimulateAnalog                  INTEGER NOT NULL,
    StickLeft                       INTEGER NOT NULL REFERENCES InputList(InputID),
    StickRight                      INTEGER NOT NULL REFERENCES InputList(InputID),
    StickUp                         INTEGER NOT NULL REFERENCES InputList(InputID),
    StickDown                       INTEGER NOT NULL REFERENCES InputList(InputID),
    Paddle0CounterClockWise         INTEGER REFERENCES InputList(InputID),
    Paddle0ClockWise                INTEGER REFERENCES InputList(InputID),
    Paddle1CounterClockWise         INTEGER REFERENCES InputList(InputID),
    Paddle1ClockWise                INTEGER REFERENCES InputList(InputID),
    BottomButton                    INTEGER REFERENCES InputList(InputID),
    TopButton                       INTEGER REFERENCES InputList(InputID),
    KeypadID                        INTEGER NOT NULL REFERENCES Keypad(KeypadID)
);

CREATE TABLE Input (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    Description                     VARCHAR,
    MachineID                       INTEGER NOT NULL REFERENCES MachineTypes(MachineID),
    MouseSpeed                      INTEGER NOT NULL,
    Player0                         INTEGER NOT NULL REFERENCES Player(PlayerID),
    Player1                         INTEGER NOT NULL REFERENCES Player(PlayerID),
    Player2                         INTEGER NOT NULL REFERENCES Player(PlayerID),
    Player3                         INTEGER NOT NULL REFERENCES Player(PlayerID),
    KeyboardID                      INTEGER REFERENCES Keyboard(KeyboardID),
    StartKey                        INTEGER REFERENCES InputList(InputID),
    SelectKey                       INTEGER REFERENCES InputList(InputID),
    OptionKey                       INTEGER REFERENCES InputList(InputID),
    CtrlKey                         INTEGER REFERENCES InputList(InputID),
    ShiftKey                        INTEGER REFERENCES InputList(InputID),
    BreakKey                        INTEGER REFERENCES InputList(InputID),
    ResetKey                        INTEGER REFERENCES InputList(InputID)
);

CREATE TABLE Sound (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    Description                     VARCHAR,
    Enabled                         INTEGER NOT NULL,
    FilterEnabled                   INTEGER NOT NULL,
    Frequency                       INTEGER NOT NULL,
    Samples                         INTEGER NOT NULL,
    Treble                          INTEGER NOT NULL,
    Bass                            INTEGER NOT NULL
);

CREATE TABLE NTSCFilter (
    NTSCID                          INTEGER PRIMARY KEY AUTOINCREMENT,
    Hue                             DECIMAL(2,1) NOT NULL,
    Saturation                      DECIMAL(2,1) NOT NULL,
    Brightness                      DECIMAL(2,1) NOT NULL,
    Contrast                        DECIMAL(2,1) NOT NULL,
    Sharpness                       DECIMAL(2,1) NOT NULL,
    Gamma                           DECIMAL(2,1) NOT NULL,
    Resolution                      DECIMAL(2,1) NOT NULL,
    Artifacts                       DECIMAL(2,1) NOT NULL,
    Fringing                        DECIMAL(2,1) NOT NULL,
    Bleed                           DECIMAL(2,1) NOT NULL,
    HueWarping                      DECIMAL(2,1) NOT NULL,
    MergeFields                     INTEGER NOT NULL,
    DMEnabled                       INTEGER NOT NULL,
    DMValue0                        DECIMAL(2,1) NOT NULL,
    DMValue1                        DECIMAL(2,1) NOT NULL,
    DMValue2                        DECIMAL(2,1) NOT NULL,
    DMValue3                        DECIMAL(2,1) NOT NULL,
    DMValue4                        DECIMAL(2,1) NOT NULL,
    DMValue5                        DECIMAL(2,1) NOT NULL
);

CREATE TABLE Video (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    Description                     VARCHAR,
    NTSCPalette                     VARCHAR,
    PALPalette                      VARCHAR,
    Fullscreen                      INTEGER NOT NULL,
    Width                           INTEGER NOT NULL,
    Height                          INTEGER NOT NULL,
    PixelDepth                      INTEGER NOT NULL,
    Zoom                            INTEGER NOT NULL,
    WideScreen                      INTEGER NOT NULL,
    HiResArtifacts                  INTEGER NOT NULL,
    NTSCFilterEnabled               INTEGER NOT NULL,
    NTSCFilterID                    INTEGER NOT NULL REFERENCES NTSCFilter(NTSCID)
);

CREATE TABLE Wizard (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    Scan                            INTEGER NOT NULL,
    ScanSubs                        INTEGER NOT NULL,
    ScanBios                        INTEGER NOT NULL,
    Directory                       VARCHAR
);

CREATE TABLE States (
    Name                            VARCHAR(100) UNIQUE NOT NULL,
    MediaCRC                        INTEGER,
    MachineID                       INTEGER NOT NULL REFERENCES MachineTypes(MachineID),
    StatesSioID                     INTEGER NOT NULL REFERENCES StatesSio(StatesSioID),
    RamSize                         INTEGER NOT NULL,
    CartLoaded                      INTEGER NOT NULL,
    BasicEnabled                    INTEGER NOT NULL,
    OptionKeyPressed                INTEGER NOT NULL,
    StartKeyPressed                 INTEGER NOT NULL,
    PortB                           INTEGER NOT NULL,
    MemImage                        INTEGER NOT NULL,
    CPU_PC                          INTEGER NOT NULL,
    CPU_A                           INTEGER NOT NULL,
    CPU_X                           INTEGER NOT NULL,
    CPU_Y                           INTEGER NOT NULL,
    CPU_STATUS                      INTEGER NOT NULL,
    CPU_STACKPTR                    INTEGER NOT NULL,
    CPU_STACK                       BLOB NOT NULL,
    ANTIC_MSC                       INTEGER NOT NULL,
    ANTIC_DMACTL                    INTEGER NOT NULL,
    ANTIC_CHACTL                    INTEGER NOT NULL,
    ANTIC_DLISTL                    INTEGER NOT NULL,
    ANTIC_DLISTH                    INTEGER NOT NULL,
    ANTIC_HSCROL                    INTEGER NOT NULL,
    ANTIC_VSCROL                    INTEGER NOT NULL,
    ANTIC_PMBASE                    INTEGER NOT NULL,
    ANTIC_CHBASE                    INTEGER NOT NULL,
    ANTIC_VCOUNT                    INTEGER NOT NULL,
    ANTIC_PENH                      INTEGER NOT NULL,
    ANTIC_PENV                      INTEGER NOT NULL,
    ANTIC_NMIEN                     INTEGER NOT NULL,
    ANTIC_NMIST                     INTEGER NOT NULL,
    GTIA_HPOSP0                     INTEGER NOT NULL,
    GTIA_HPOSP1                     INTEGER NOT NULL,
    GTIA_HPOSP2                     INTEGER NOT NULL,
    GTIA_HPOSP3                     INTEGER NOT NULL,
    GTIA_HPOSM0                     INTEGER NOT NULL,
    GTIA_HPOSM1                     INTEGER NOT NULL,
    GTIA_HPOSM2                     INTEGER NOT NULL,
    GTIA_HPOSM3                     INTEGER NOT NULL,
    GTIA_SIZEP0                     INTEGER NOT NULL,
    GTIA_SIZEP1                     INTEGER NOT NULL,
    GTIA_SIZEP2                     INTEGER NOT NULL,
    GTIA_SIZEP3                     INTEGER NOT NULL,
    GTIA_SIZEM                      INTEGER NOT NULL,
    GTIA_GRAFP0                     INTEGER NOT NULL,
    GTIA_GRAFP1                     INTEGER NOT NULL,
    GTIA_GRAFP2                     INTEGER NOT NULL,
    GTIA_GRAFP3                     INTEGER NOT NULL,
    GTIA_GRAFM                      INTEGER NOT NULL,
    GTIA_COLPF0                     INTEGER NOT NULL,
    GTIA_COLPF1                     INTEGER NOT NULL,
    GTIA_COLPF2                     INTEGER NOT NULL,
    GTIA_COLPF3                     INTEGER NOT NULL,
    GTIA_COLPM0                     INTEGER NOT NULL,
    GTIA_COLPM1                     INTEGER NOT NULL,
    GTIA_COLPM2                     INTEGER NOT NULL,
    GTIA_COLPM3                     INTEGER NOT NULL,
    GTIA_COLBK                      INTEGER NOT NULL,
    GTIA_M0PF                       INTEGER NOT NULL,
    GTIA_M1PF                       INTEGER NOT NULL,
    GTIA_M2PF                       INTEGER NOT NULL,
    GTIA_M3PF                       INTEGER NOT NULL,
    GTIA_P0PF                       INTEGER NOT NULL,
    GTIA_P1PF                       INTEGER NOT NULL,
    GTIA_P2PF                       INTEGER NOT NULL,
    GTIA_P3PF                       INTEGER NOT NULL,
    GTIA_M0PL                       INTEGER NOT NULL,
    GTIA_M1PL                       INTEGER NOT NULL,
    GTIA_M2PL                       INTEGER NOT NULL,
    GTIA_M3PL                       INTEGER NOT NULL,
    GTIA_P0PL                       INTEGER NOT NULL,
    GTIA_P1PL                       INTEGER NOT NULL,
    GTIA_P2PL                       INTEGER NOT NULL,
    GTIA_P3PL                       INTEGER NOT NULL,
    GTIA_TRIG0                      INTEGER NOT NULL,
    GTIA_TRIG1                      INTEGER NOT NULL,
    GTIA_TRIG2                      INTEGER NOT NULL,
    GTIA_TRIG3                      INTEGER NOT NULL,
    GTIA_PRIOR                      INTEGER NOT NULL,
    GTIA_GRACTL                     INTEGER NOT NULL,
    GTIA_VDELAY                     INTEGER NOT NULL,
    GTIA_PAL                        INTEGER NOT NULL,
    GTIA_CONSOL                     INTEGER NOT NULL,
    POKEY_AUDF1                     INTEGER NOT NULL,
    POKEY_AUDF2                     INTEGER NOT NULL,
    POKEY_AUDF3                     INTEGER NOT NULL,
    POKEY_AUDF4                     INTEGER NOT NULL,
    POKEY_AUDC1                     INTEGER NOT NULL,
    POKEY_AUDC2                     INTEGER NOT NULL,
    POKEY_AUDC3                     INTEGER NOT NULL,
    POKEY_AUDC4                     INTEGER NOT NULL,
    POKEY_AUDCTL                    INTEGER NOT NULL,
    POKEY_STIMER                    INTEGER NOT NULL,
    POKEY_SKSTAT                    INTEGER NOT NULL,
    POKEY_SKCTL                     INTEGER NOT NULL,
    POKEY_SEROUT                    INTEGER NOT NULL,
    POKEY_SERIN                     INTEGER NOT NULL,
    POKEY_IRQST                     INTEGER NOT NULL,
    POKEY_IRQEN                     INTEGER NOT NULL,
    POKEY_KBCODE                    INTEGER NOT NULL,
    POKEY_POT0                      INTEGER NOT NULL,
	POKEY_POT1                      INTEGER NOT NULL,
    POKEY_POT2                      INTEGER NOT NULL,
    POKEY_POT3                      INTEGER NOT NULL,
    POKEY_POT4                      INTEGER NOT NULL,
    POKEY_POT5                      INTEGER NOT NULL,
    POKEY_POT6                      INTEGER NOT NULL,
    POKEY_POT7                      INTEGER NOT NULL,
    POKEY_ALLPOT                    INTEGER NOT NULL,
    PIA_PORTA                       INTEGER NOT NULL,
    PIA_PORTB                       INTEGER NOT NULL,
    PIA_PORTA_DIR                   INTEGER NOT NULL,
    PIA_PORTB_DIR                   INTEGER NOT NULL,
    PIA_PACTL                       INTEGER NOT NULL,
    PIA_PBCTL                       INTEGER NOT NULL,
    Memory                          BLOB NOT NULL,
    BankSwitchCart                  BLOB,
    XEBankedRAM                     BLOB,
    BIOS                            BLOB,
    BASIC                           BLOB
);

CREATE TABLE StatesSio (
    StatesSioID                     INTEGER PRIMARY KEY AUTOINCREMENT,
    CurrentDevice                   INTEGER NOT NULL,
    MotorOn                         INTEGER NOT NULL,
    MotorOnTime                     INTEGER NOT NULL,
    CmdFrame                        BLOB NOT NULL,
    CmdIndex                        INTEGER NOT NULL,
    Disk0                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk1                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk2                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk3                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk4                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk5                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk6                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Disk7                           INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID),
    Cas                             INTEGER NOT NULL REFERENCES StatesSioDevices(StatesSioDevicesID)
);

CREATE TABLE StatesSioDevices (
    StatesSioDevicesID              INTEGER PRIMARY KEY AUTOINCREMENT,
    Filename                        VARCHAR,
    Fbuffer                         BLOB,
    BufferSize                      INTEGER NOT NULL,
    NumBlocks                       INTEGER NOT NULL,
    CurrentBlock                    INTEGER NOT NULL,
    CurrentByte                     INTEGER NOT NULL,
    ExpectedBytes                   INTEGER NOT NULL,
    Header                          BLOB,
    DataFrame                       BLOB,
    HeaderSize                      INTEGER NOT NULL,
    Baud                            INTEGER NOT NULL,
    CurrentData                     INTEGER,
    ActionTime                      INTEGER NOT NULL,
    Status                          INTEGER NOT NULL,
    Type                            INTEGER NOT NULL,
    WriteProtect                    INTEGER,
    SectorSize                      INTEGER,
    DskType                         INTEGER,
    Blk                             BLOB,
    RecordPressed                   INTEGER,
    MotorGapDelay                   INTEGER,
    Description                     VARCHAR
);

CREATE TABLE Media (
    Title                           VARCHAR NOT NULL,
    Filename                        VARCHAR,
    ImageSize                       INTEGER NOT NULL,
    CRC                             INTEGER UNIQUE NOT NULL,
    MD5                             VARCHAR(40),
    SHA1                            VARCHAR(50),
    MachineID                       INTEGER NOT NULL REFERENCES MachineTypes(MachineID),
    Mapping                         INTEGER NOT NULL,
    MediaType                       INTEGER NOT NULL,
    InputProfile                    VARCHAR(100),
    VideoProfile                    VARCHAR(100),
    SoundProfile                    VARCHAR(100),
    WriteProtect                    INTEGER NOT NULL,
    BasicEnable                     INTEGER NOT NULL,
    SIOPatchEnable                  INTEGER NOT NULL,
    Favorite                        INTEGER NOT NULL,
    RamSize                         INTEGER NOT NULL,
    Flags                           INTEGER NOT NULL
);

CREATE TABLE CurrentMedia (
    Name                            VARCHAR(50) UNIQUE NOT NULL,
    Mapping                         INTEGER NOT NULL,
    Image                           BLOB
);

CREATE TABLE Palette (
    Name                            VARCHAR(50) UNIQUE NOT NULL,
    Description                     VARCHAR,
    C00                             INTEGER NOT NULL,
    C01                             INTEGER NOT NULL,
    C02                             INTEGER NOT NULL,
    C03                             INTEGER NOT NULL,
    C04                             INTEGER NOT NULL,
    C05                             INTEGER NOT NULL,
    C06                             INTEGER NOT NULL,
    C07                             INTEGER NOT NULL,
    C08                             INTEGER NOT NULL,
    C09                             INTEGER NOT NULL,
    C0a                             INTEGER NOT NULL,
    C0b                             INTEGER NOT NULL,
    C0c                             INTEGER NOT NULL,
    C0d                             INTEGER NOT NULL,
    C0e                             INTEGER NOT NULL,
    C0f                             INTEGER NOT NULL,
    C10                             INTEGER NOT NULL,
    C11                             INTEGER NOT NULL,
    C12                             INTEGER NOT NULL,
    C13                             INTEGER NOT NULL,
    C14                             INTEGER NOT NULL,
    C15                             INTEGER NOT NULL,
    C16                             INTEGER NOT NULL,
    C17                             INTEGER NOT NULL,
    C18                             INTEGER NOT NULL,
    C19                             INTEGER NOT NULL,
    C1a                             INTEGER NOT NULL,
    C1b                             INTEGER NOT NULL,
    C1c                             INTEGER NOT NULL,
    C1d                             INTEGER NOT NULL,
    C1e                             INTEGER NOT NULL,
    C1f                             INTEGER NOT NULL,
    C20                             INTEGER NOT NULL,
    C21                             INTEGER NOT NULL,
    C22                             INTEGER NOT NULL,
    C23                             INTEGER NOT NULL,
    C24                             INTEGER NOT NULL,
    C25                             INTEGER NOT NULL,
    C26                             INTEGER NOT NULL,
    C27                             INTEGER NOT NULL,
    C28                             INTEGER NOT NULL,
    C29                             INTEGER NOT NULL,
    C2a                             INTEGER NOT NULL,
    C2b                             INTEGER NOT NULL,
    C2c                             INTEGER NOT NULL,
    C2d                             INTEGER NOT NULL,
    C2e                             INTEGER NOT NULL,
    C2f                             INTEGER NOT NULL,
    C30                             INTEGER NOT NULL,
    C31                             INTEGER NOT NULL,
    C32                             INTEGER NOT NULL,
    C33                             INTEGER NOT NULL,
    C34                             INTEGER NOT NULL,
    C35                             INTEGER NOT NULL,
    C36                             INTEGER NOT NULL,
    C37                             INTEGER NOT NULL,
    C38                             INTEGER NOT NULL,
    C39                             INTEGER NOT NULL,
    C3a                             INTEGER NOT NULL,
    C3b                             INTEGER NOT NULL,
    C3c                             INTEGER NOT NULL,
    C3d                             INTEGER NOT NULL,
    C3e                             INTEGER NOT NULL,
    C3f                             INTEGER NOT NULL,
    C40                             INTEGER NOT NULL,
    C41                             INTEGER NOT NULL,
    C42                             INTEGER NOT NULL,
    C43                             INTEGER NOT NULL,
    C44                             INTEGER NOT NULL,
    C45                             INTEGER NOT NULL,
    C46                             INTEGER NOT NULL,
    C47                             INTEGER NOT NULL,
    C48                             INTEGER NOT NULL,
    C49                             INTEGER NOT NULL,
    C4a                             INTEGER NOT NULL,
    C4b                             INTEGER NOT NULL,
    C4c                             INTEGER NOT NULL,
    C4d                             INTEGER NOT NULL,
    C4e                             INTEGER NOT NULL,
    C4f                             INTEGER NOT NULL,
    C50                             INTEGER NOT NULL,
    C51                             INTEGER NOT NULL,
    C52                             INTEGER NOT NULL,
    C53                             INTEGER NOT NULL,
    C54                             INTEGER NOT NULL,
    C55                             INTEGER NOT NULL,
    C56                             INTEGER NOT NULL,
    C57                             INTEGER NOT NULL,
    C58                             INTEGER NOT NULL,
    C59                             INTEGER NOT NULL,
    C5a                             INTEGER NOT NULL,
    C5b                             INTEGER NOT NULL,
    C5c                             INTEGER NOT NULL,
    C5d                             INTEGER NOT NULL,
    C5e                             INTEGER NOT NULL,
    C5f                             INTEGER NOT NULL,
    C60                             INTEGER NOT NULL,
    C61                             INTEGER NOT NULL,
    C62                             INTEGER NOT NULL,
    C63                             INTEGER NOT NULL,
    C64                             INTEGER NOT NULL,
    C65                             INTEGER NOT NULL,
    C66                             INTEGER NOT NULL,
    C67                             INTEGER NOT NULL,
    C68                             INTEGER NOT NULL,
    C69                             INTEGER NOT NULL,
    C6a                             INTEGER NOT NULL,
    C6b                             INTEGER NOT NULL,
    C6c                             INTEGER NOT NULL,
    C6d                             INTEGER NOT NULL,
    C6e                             INTEGER NOT NULL,
    C6f                             INTEGER NOT NULL,
    C70                             INTEGER NOT NULL,
    C71                             INTEGER NOT NULL,
    C72                             INTEGER NOT NULL,
    C73                             INTEGER NOT NULL,
    C74                             INTEGER NOT NULL,
    C75                             INTEGER NOT NULL,
    C76                             INTEGER NOT NULL,
    C77                             INTEGER NOT NULL,
    C78                             INTEGER NOT NULL,
    C79                             INTEGER NOT NULL,
    C7a                             INTEGER NOT NULL,
    C7b                             INTEGER NOT NULL,
    C7c                             INTEGER NOT NULL,
    C7d                             INTEGER NOT NULL,
    C7e                             INTEGER NOT NULL,
    C7f                             INTEGER NOT NULL,
    C80                             INTEGER NOT NULL,
    C81                             INTEGER NOT NULL,
    C82                             INTEGER NOT NULL,
    C83                             INTEGER NOT NULL,
    C84                             INTEGER NOT NULL,
    C85                             INTEGER NOT NULL,
    C86                             INTEGER NOT NULL,
    C87                             INTEGER NOT NULL,
    C88                             INTEGER NOT NULL,
    C89                             INTEGER NOT NULL,
    C8a                             INTEGER NOT NULL,
    C8b                             INTEGER NOT NULL,
    C8c                             INTEGER NOT NULL,
    C8d                             INTEGER NOT NULL,
    C8e                             INTEGER NOT NULL,
    C8f                             INTEGER NOT NULL,
    C90                             INTEGER NOT NULL,
    C91                             INTEGER NOT NULL,
    C92                             INTEGER NOT NULL,
    C93                             INTEGER NOT NULL,
    C94                             INTEGER NOT NULL,
    C95                             INTEGER NOT NULL,
    C96                             INTEGER NOT NULL,
    C97                             INTEGER NOT NULL,
    C98                             INTEGER NOT NULL,
    C99                             INTEGER NOT NULL,
    C9a                             INTEGER NOT NULL,
    C9b                             INTEGER NOT NULL,
    C9c                             INTEGER NOT NULL,
    C9d                             INTEGER NOT NULL,
    C9e                             INTEGER NOT NULL,
    C9f                             INTEGER NOT NULL,
    Ca0                             INTEGER NOT NULL,
    Ca1                             INTEGER NOT NULL,
    Ca2                             INTEGER NOT NULL,
    Ca3                             INTEGER NOT NULL,
    Ca4                             INTEGER NOT NULL,
    Ca5                             INTEGER NOT NULL,
    Ca6                             INTEGER NOT NULL,
    Ca7                             INTEGER NOT NULL,
    Ca8                             INTEGER NOT NULL,
    Ca9                             INTEGER NOT NULL,
    Caa                             INTEGER NOT NULL,
    Cab                             INTEGER NOT NULL,
    Cac                             INTEGER NOT NULL,
    Cad                             INTEGER NOT NULL,
    Cae                             INTEGER NOT NULL,
    Caf                             INTEGER NOT NULL,
    Cb0                             INTEGER NOT NULL,
    Cb1                             INTEGER NOT NULL,
    Cb2                             INTEGER NOT NULL,
    Cb3                             INTEGER NOT NULL,
    Cb4                             INTEGER NOT NULL,
    Cb5                             INTEGER NOT NULL,
    Cb6                             INTEGER NOT NULL,
    Cb7                             INTEGER NOT NULL,
    Cb8                             INTEGER NOT NULL,
    Cb9                             INTEGER NOT NULL,
    Cba                             INTEGER NOT NULL,
    Cbb                             INTEGER NOT NULL,
    Cbc                             INTEGER NOT NULL,
    Cbd                             INTEGER NOT NULL,
    Cbe                             INTEGER NOT NULL,
    Cbf                             INTEGER NOT NULL,
    Cc0                             INTEGER NOT NULL,
    Cc1                             INTEGER NOT NULL,
    Cc2                             INTEGER NOT NULL,
    Cc3                             INTEGER NOT NULL,
    Cc4                             INTEGER NOT NULL,
    Cc5                             INTEGER NOT NULL,
    Cc6                             INTEGER NOT NULL,
    Cc7                             INTEGER NOT NULL,
    Cc8                             INTEGER NOT NULL,
    Cc9                             INTEGER NOT NULL,
    Cca                             INTEGER NOT NULL,
    Ccb                             INTEGER NOT NULL,
    Ccc                             INTEGER NOT NULL,
    Ccd                             INTEGER NOT NULL,
    Cce                             INTEGER NOT NULL,
    Ccf                             INTEGER NOT NULL,
    Cd0                             INTEGER NOT NULL,
    Cd1                             INTEGER NOT NULL,
    Cd2                             INTEGER NOT NULL,
    Cd3                             INTEGER NOT NULL,
    Cd4                             INTEGER NOT NULL,
    Cd5                             INTEGER NOT NULL,
    Cd6                             INTEGER NOT NULL,
    Cd7                             INTEGER NOT NULL,
    Cd8                             INTEGER NOT NULL,
    Cd9                             INTEGER NOT NULL,
    Cda                             INTEGER NOT NULL,
    Cdb                             INTEGER NOT NULL,
    Cdc                             INTEGER NOT NULL,
    Cdd                             INTEGER NOT NULL,
    Cde                             INTEGER NOT NULL,
    Cdf                             INTEGER NOT NULL,
    Ce0                             INTEGER NOT NULL,
    Ce1                             INTEGER NOT NULL,
    Ce2                             INTEGER NOT NULL,
    Ce3                             INTEGER NOT NULL,
    Ce4                             INTEGER NOT NULL,
    Ce5                             INTEGER NOT NULL,
    Ce6                             INTEGER NOT NULL,
    Ce7                             INTEGER NOT NULL,
    Ce8                             INTEGER NOT NULL,
    Ce9                             INTEGER NOT NULL,
    Cea                             INTEGER NOT NULL,
    Ceb                             INTEGER NOT NULL,
    Cec                             INTEGER NOT NULL,
    Ced                             INTEGER NOT NULL,
    Cee                             INTEGER NOT NULL,
    Cef                             INTEGER NOT NULL,
    Cf0                             INTEGER NOT NULL,
    Cf1                             INTEGER NOT NULL,
    Cf2                             INTEGER NOT NULL,
    Cf3                             INTEGER NOT NULL,
    Cf4                             INTEGER NOT NULL,
    Cf5                             INTEGER NOT NULL,
    Cf6                             INTEGER NOT NULL,
    Cf7                             INTEGER NOT NULL,
    Cf8                             INTEGER NOT NULL,
    Cf9                             INTEGER NOT NULL,
    Cfa                             INTEGER NOT NULL,
    Cfb                             INTEGER NOT NULL,
    Cfc                             INTEGER NOT NULL,
    Cfd                             INTEGER NOT NULL,
    Cfe                             INTEGER NOT NULL,
    Cff                             INTEGER NOT NULL
);
-------------------------------------------------------------------------------
--                                Initial Data                               --
-------------------------------------------------------------------------------
-- These value ID's match directly to the e_machine_type enum in console.h
INSERT INTO MachineTypes VALUES (0, "Auto");
INSERT INTO MachineTypes VALUES (1, "Atari 5200");
INSERT INTO MachineTypes VALUES (2, "Atari 800");
INSERT INTO MachineTypes VALUES (3, "Atari XL/XE");

-- These value ID's match directly to the IDs in gtia.h
INSERT INTO VideoSystemTypes VALUES (1, "PAL");
INSERT INTO VideoSystemTypes VALUES (15, "NTSC");

-- These value ID's match directly to the e_media_type enum in cart.h
INSERT INTO MediaTypes VALUES (0, "Auto");
INSERT INTO MediaTypes VALUES (1, "32K Standard");
INSERT INTO MediaTypes VALUES (2, "16K Two Chip");
INSERT INTO MediaTypes VALUES (3, "16K One Chip");
INSERT INTO MediaTypes VALUES (4, "40K Bounty Bob");
INSERT INTO MediaTypes VALUES (5, "64K Bank Switch");
INSERT INTO MediaTypes VALUES (6, "8K Standard");
INSERT INTO MediaTypes VALUES (7, "4K Standard");
INSERT INTO MediaTypes VALUES (8, "8K Standard");
INSERT INTO MediaTypes VALUES (9, "16K Standard");
INSERT INTO MediaTypes VALUES (10, "32K Standard");
INSERT INTO MediaTypes VALUES (11, "40K Bounty Bob");
INSERT INTO MediaTypes VALUES (12, "8K Right Slot");
INSERT INTO MediaTypes VALUES (13, "XEGS Bank Switch");
INSERT INTO MediaTypes VALUES (14, "SWXEGS Bank Switch");
INSERT INTO MediaTypes VALUES (15, "MegaCart");
INSERT INTO MediaTypes VALUES (16, "OSS Super Cart");
INSERT INTO MediaTypes VALUES (17, "OSS2 Super Cart");
INSERT INTO MediaTypes VALUES (18, "ATMax Cart");
INSERT INTO MediaTypes VALUES (19, "ATRax Cart");
INSERT INTO MediaTypes VALUES (20, "WILL Cart");
INSERT INTO MediaTypes VALUES (21, "SDX Cart");
INSERT INTO MediaTypes VALUES (22, "EXP Cart");
INSERT INTO MediaTypes VALUES (23, "Diamond Cart");
INSERT INTO MediaTypes VALUES (24, "Phoenix Cart");
INSERT INTO MediaTypes VALUES (25, "Blizzard Cart");
INSERT INTO MediaTypes VALUES (26, "Cart");
INSERT INTO MediaTypes VALUES (27, "Floppy");
INSERT INTO MediaTypes VALUES (28, "Cassette");
INSERT INTO MediaTypes VALUES (29, "Executable");

-- These value ID's match directly to the e_ctrlr_type enum in input.h
INSERT INTO ControllerTypes VALUES (0, "None");
INSERT INTO ControllerTypes VALUES (1, "Joystick");
INSERT INTO ControllerTypes VALUES (2, "Trackball");
INSERT INTO ControllerTypes VALUES (3, "Paddles");
INSERT INTO ControllerTypes VALUES (4, "Keypad");

-- These value ID's match directly to the defines in pokey.h
--    These strings match directly to the columns for Table Keyboard
INSERT INTO KeyboardCodes VALUES (0, "KEY_L");
INSERT INTO KeyboardCodes VALUES (1, "KEY_J");
INSERT INTO KeyboardCodes VALUES (2, "KEY_SEMI");
INSERT INTO KeyboardCodes VALUES (5, "KEY_K");
INSERT INTO KeyboardCodes VALUES (6, "KEY_PLUS");
INSERT INTO KeyboardCodes VALUES (7, "KEY_STAR");
INSERT INTO KeyboardCodes VALUES (8, "KEY_O");
INSERT INTO KeyboardCodes VALUES (10, "KEY_P");
INSERT INTO KeyboardCodes VALUES (11, "KEY_U");
INSERT INTO KeyboardCodes VALUES (12, "KEY_RETURN");
INSERT INTO KeyboardCodes VALUES (13, "KEY_I");
INSERT INTO KeyboardCodes VALUES (14, "KEY_MINUS");
INSERT INTO KeyboardCodes VALUES (15, "KEY_EQUALS");
INSERT INTO KeyboardCodes VALUES (16, "KEY_V");
INSERT INTO KeyboardCodes VALUES (17, "KEY_HELP");
INSERT INTO KeyboardCodes VALUES (18, "KEY_C");
INSERT INTO KeyboardCodes VALUES (21, "KEY_B");
INSERT INTO KeyboardCodes VALUES (22, "KEY_X");
INSERT INTO KeyboardCodes VALUES (23, "KEY_Z");
INSERT INTO KeyboardCodes VALUES (24, "KEY_4");
INSERT INTO KeyboardCodes VALUES (26, "KEY_3");
INSERT INTO KeyboardCodes VALUES (27, "KEY_6");
INSERT INTO KeyboardCodes VALUES (28, "KEY_ESC");
INSERT INTO KeyboardCodes VALUES (29, "KEY_5");
INSERT INTO KeyboardCodes VALUES (30, "KEY_2");
INSERT INTO KeyboardCodes VALUES (31, "KEY_1");
INSERT INTO KeyboardCodes VALUES (32, "KEY_COMMA");
INSERT INTO KeyboardCodes VALUES (33, "KEY_SPACE");
INSERT INTO KeyboardCodes VALUES (34, "KEY_PERIOD");
INSERT INTO KeyboardCodes VALUES (35, "KEY_N");
INSERT INTO KeyboardCodes VALUES (37, "KEY_M");
INSERT INTO KeyboardCodes VALUES (38, "KEY_SLASH");
INSERT INTO KeyboardCodes VALUES (39, "KEY_FUJI");
INSERT INTO KeyboardCodes VALUES (40, "KEY_R");
INSERT INTO KeyboardCodes VALUES (42, "KEY_E");
INSERT INTO KeyboardCodes VALUES (43, "KEY_Y");
INSERT INTO KeyboardCodes VALUES (44, "KEY_TAB");
INSERT INTO KeyboardCodes VALUES (45, "KEY_T");
INSERT INTO KeyboardCodes VALUES (46, "KEY_W");
INSERT INTO KeyboardCodes VALUES (47, "KEY_Q");
INSERT INTO KeyboardCodes VALUES (48, "KEY_9");
INSERT INTO KeyboardCodes VALUES (50, "KEY_0");
INSERT INTO KeyboardCodes VALUES (51, "KEY_7");
INSERT INTO KeyboardCodes VALUES (52, "KEY_BKSP");
INSERT INTO KeyboardCodes VALUES (53, "KEY_8");
INSERT INTO KeyboardCodes VALUES (54, "KEY_LESST");
INSERT INTO KeyboardCodes VALUES (55, "KEY_MORET");
INSERT INTO KeyboardCodes VALUES (56, "KEY_F");
INSERT INTO KeyboardCodes VALUES (57, "KEY_H");
INSERT INTO KeyboardCodes VALUES (58, "KEY_D");
INSERT INTO KeyboardCodes VALUES (60, "KEY_CAPS");
INSERT INTO KeyboardCodes VALUES (61, "KEY_G");
INSERT INTO KeyboardCodes VALUES (62, "KEY_S");
INSERT INTO KeyboardCodes VALUES (63, "KEY_A");

-- These value ID's match directly to the defines in pokey.h
--    These strings match directly to the columns for Table Keypad
INSERT INTO KeypadCodes VALUES (3, "KEY_POUND");
INSERT INTO KeypadCodes VALUES (5, "KEY_0");
INSERT INTO KeypadCodes VALUES (7, "KEY_STAR");
INSERT INTO KeypadCodes VALUES (9, "KEY_RESET");
INSERT INTO KeypadCodes VALUES (11, "KEY_9");
INSERT INTO KeypadCodes VALUES (13, "KEY_8");
INSERT INTO KeypadCodes VALUES (15, "KEY_7");
INSERT INTO KeypadCodes VALUES (17, "KEY_PAUSE");
INSERT INTO KeypadCodes VALUES (19, "KEY_6");
INSERT INTO KeypadCodes VALUES (21, "KEY_5");
INSERT INTO KeypadCodes VALUES (23, "KEY_4");
INSERT INTO KeypadCodes VALUES (25, "KEY_START");
INSERT INTO KeypadCodes VALUES (27, "KEY_3");
INSERT INTO KeypadCodes VALUES (29, "KEY_2");
INSERT INTO KeypadCodes VALUES (31, "KEY_1");

-- These value ID's match directly to the e_dev_type enum in input.h
INSERT INTO InputDevices VALUES (0, "Keyboard");
INSERT INTO InputDevices VALUES (1, "Joystick");
INSERT INTO InputDevices VALUES (2, "Mouse");

-- These value ID's match directly to the e_part_type enum in input.h
INSERT INTO InputParts VALUES (0, "D-Pad / Hat");
INSERT INTO InputParts VALUES (1, "Stick Axis");
INSERT INTO InputParts VALUES (2, "Track Ball");
INSERT INTO InputParts VALUES (3, "Button");
INSERT INTO InputParts VALUES (4, "Key");

-- These value ID's match directly to the e_direction enum in input.h
INSERT INTO InputDirection VALUES (0, "Left");
INSERT INTO InputDirection VALUES (1, "Right");
INSERT INTO InputDirection VALUES (2, "Up");
INSERT INTO InputDirection VALUES (3, "Down");
INSERT INTO InputDirection VALUES (4, "None");
INSERT INTO InputDirection VALUES (5, "Plus");
INSERT INTO InputDirection VALUES (6, "Minus");
INSERT INTO InputDirection VALUES (7, "Press");
INSERT INTO InputDirection VALUES (8, "Release");

-------------------------------------------------------------------------------
--                          Default Configuration                            --
-------------------------------------------------------------------------------
INSERT INTO Config VALUES (
    "Default", "","", "5200.bin","","","","default.bmp","",
    "Default 5200 Settings","Default 800 Settings","Default","Default",
    "Default",1,15,100,
    16,48,64,
    1,0,0,0,0,0,1,0,1
);

INSERT INTO CurrentMedia VALUES ( "Cart", 0, zeroblob(8388608) );

INSERT INTO Sound VALUES ( "Default", "", 1, 0, 44100, 1024, -8, 16 );

INSERT INTO NTSCFilter VALUES ( NULL, 0, -0.1, 0, 0, -0.5, 0, 0, 0.4, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 );
INSERT INTO Video VALUES ( "Default", "", "Default", "DefaultPal", 0, 0, 0, 8, 0, 0, 0, 0, (SELECT max(NTSCID) FROM NTSCFilter) );
INSERT INTO Wizard VALUES ( "5200", 1, 1, 1, "" );
INSERT INTO Wizard VALUES ( "800", 1, 1, 1, "" );
INSERT INTO Wizard VALUES ( "Bios", 1, 1, 1, "" );

-------------------------------------------------------------------------------
--                          Default User Key List
-------------------------------------------------------------------------------
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 285, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 27, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 13, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 115, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 49, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 50, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 51, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 52, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 53, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 54, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 55, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 56, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 768, 4, 57, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 49, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 50, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 51, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 52, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 53, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 54, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 55, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 56, 7 );
INSERT INTO InputList VALUES ( NULL, 0, 3, 4, 57, 7 );

INSERT INTO UIKeys VALUES ( 
    "Default", 
    "", 
	((SELECT max(InputID) FROM InputList) - 21),
	((SELECT max(InputID) FROM InputList) - 20),
	((SELECT max(InputID) FROM InputList) - 19),
	((SELECT max(InputID) FROM InputList) - 18),
	((SELECT max(InputID) FROM InputList) - 17),
	((SELECT max(InputID) FROM InputList) - 16),
	((SELECT max(InputID) FROM InputList) - 15),
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

-------------------------------------------------------------------------------
--                          Default 5200 Input                               --
-------------------------------------------------------------------------------
-- Player 0 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 291, 7 ); --KEY_START = F10
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 292, 7 ); --KEY_PAUSE = F11
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 293, 7 ); --KEY_RESET = F12
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 268, 7 ); --KEY_STAR = KP_MULTIPLY
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 267, 7 ); --KEY_POUND = KP_DIVIDE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 256, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 257, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 258, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 259, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 260, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 261, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 262, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 263, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 264, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 265, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --StickLeft = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --StickRight = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --StickUp = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --StickDown = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 13, 7 ); --Bottom Button = RETURN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 303, 7 ); --Top Button = RSHIFT

INSERT INTO Player VALUES ( NULL, 1, 20, 96, 0, 
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
    NULL, NULL, NULL, NULL, 
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 1 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 284, 7 ); --KEY_START = F3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 285, 7 ); --KEY_PAUSE = F4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 286, 7 ); --KEY_RESET = F5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 113, 7 ); --KEY_STAR = q
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 9, 7 ); --KEY_POUND = TAB
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 48, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 49, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 50, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 51, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 52, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 53, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 54, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 55, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 56, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 57, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --StickLeft = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --StickRight = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --StickUp = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --StickDown = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 308, 7 ); --Bottom Button = LCTRL
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 304, 7 ); --Top Button = LSHIFT

INSERT INTO Player VALUES ( NULL, 1, 20, 96, 0, 
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
    NULL, NULL, NULL, NULL, 
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 2 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 291, 7 ); --KEY_START = F10
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 292, 7 ); --KEY_PAUSE = F11
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 293, 7 ); --KEY_RESET = F12
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 268, 7 ); --KEY_STAR = KP_MULTIPLY
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 267, 7 ); --KEY_POUND = KP_DIVIDE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 256, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 257, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 258, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 259, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 260, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 261, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 262, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 263, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 264, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 265, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --StickLeft = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --StickRight = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --StickUp = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --StickDown = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 13, 7 ); --Bottom Button = RETURN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 303, 7 ); --Top Button = RSHIFT

INSERT INTO Player VALUES ( NULL, 0, 20, 96, 0, 
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
    NULL, NULL, NULL, NULL, 
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 3 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 284, 7 ); --KEY_START = F3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 285, 7 ); --KEY_PAUSE = F4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 286, 7 ); --KEY_RESET = F5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 113, 7 ); --KEY_STAR = q
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 9, 7 ); --KEY_POUND = TAB
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 48, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 49, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 50, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 51, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 52, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 53, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 54, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 55, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 56, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 57, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --StickLeft = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --StickRight = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --StickUp = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --StickDown = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 308, 7 ); --Bottom Button = LCTRL
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 304, 7 ); --Top Button = LSHIFT

INSERT INTO Player VALUES ( NULL, 0, 20, 96, 0, 
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
    NULL, NULL, NULL, NULL, 
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	(SELECT max(KeypadID) FROM Keypad)
);

INSERT INTO Input VALUES ( 
    "Default 5200 Settings", "", 1, 3, 
	(SELECT max(PlayerID) FROM Player) - 3,
	(SELECT max(PlayerID) FROM Player) - 2,
	(SELECT max(PlayerID) FROM Player) - 1,
	(SELECT max(PlayerID) FROM Player),
    NULL, NULL, NULL, NULL, NULL, NULL,NULL,NULL
);

-------------------------------------------------------------------------------
--                          Default 800 Input                                --
-------------------------------------------------------------------------------
-- Keyboard --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 108, 7 ); --KEY_L = l
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 106, 7 ); --KEY_J = j
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 59, 7 ); --KEY_SEMI = SEMICOLON
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 107, 7 ); --KEY_K = k
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 270, 7 ); --KEY_PLUS = KP_PLUS
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 268, 7 ); --KEY_STAR = KP_MULTIPLY
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 111, 7 ); --KEY_O = o
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 112, 7 ); --KEY_P = p
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 117, 7 ); --KEY_U = u
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 13, 7 ); --KEY_RETURN = RETURN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 105, 7 ); --KEY_I = i
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 45, 7 ); --KEY_MINUS = MINUS
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 61, 7 ); --KEY_EQUALS = EQUALS
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 118, 7 ); --KEY_V = v
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 290, 7 ); --KEY_HELP = F9
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 99, 7 ); --KEY_C = c
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 98, 7 ); --KEY_B = b
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --KEY_X = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 122, 7 ); --KEY_Z = z
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 52, 7 ); --KEY_4 = 4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 51, 7 ); --KEY_3 = 3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 54, 7 ); --KEY_6 = 6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 283, 7 ); --KEY_ESC = F2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 53, 7 ); --KEY_5 = 5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 50, 7 ); --KEY_2 = 2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 49, 7 ); --KEY_1 = 1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 44, 7 ); --KEY_COMMA = COMMA
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 32, 7 ); --KEY_SPACE = SPACE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 46, 7 ); --KEY_PERIOD = PERIOD
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 110, 7 ); --KEY_N = n
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 109, 7 ); --KEY_M = m
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 47, 7 ); --KEY_SLASH = SLASH
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 278, 7 ); --KEY_FUJI = HOME
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 114, 7 ); --KEY_R = r
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 101, 7 ); --KEY_E = e
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 121, 7 ); --KEY_Y = y
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 9, 7 ); --KEY_TAB = TAB
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 116, 7 ); --KEY_T = t
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --KEY_W = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 113, 7 ); --KEY_Q = q
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 57, 7 ); --KEY_9 = 9
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 48, 7 ); --KEY_0 = 0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 55, 7 ); --KEY_7 = 7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 8, 7 ); --KEY_BKSP = BACKSPACE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 56, 7 ); --KEY_8 = 8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 44, 7 ); --KEY_LESST = COMMA
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 46, 7 ); --KEY_MORET = PERIOD
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 102, 7 ); --KEY_F = f
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 104, 7 ); --KEY_H = h
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --KEY_D = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 301, 7 ); --KEY_CAPS = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 103, 7 ); --KEY_G = g
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 115, 7 ); --KEY_S = s
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --KEY_A = a

INSERT INTO Keyboard VALUES (
    NULL,
	((SELECT max(InputID) FROM InputList) - 53),
	((SELECT max(InputID) FROM InputList) - 52),
	((SELECT max(InputID) FROM InputList) - 51),
	((SELECT max(InputID) FROM InputList) - 50),
	((SELECT max(InputID) FROM InputList) - 49),
	((SELECT max(InputID) FROM InputList) - 48),
	((SELECT max(InputID) FROM InputList) - 47),
	((SELECT max(InputID) FROM InputList) - 46),
	((SELECT max(InputID) FROM InputList) - 45),
	((SELECT max(InputID) FROM InputList) - 44),
	((SELECT max(InputID) FROM InputList) - 43),
	((SELECT max(InputID) FROM InputList) - 42),
	((SELECT max(InputID) FROM InputList) - 41),
	((SELECT max(InputID) FROM InputList) - 40),
	((SELECT max(InputID) FROM InputList) - 39),
	((SELECT max(InputID) FROM InputList) - 38),
	((SELECT max(InputID) FROM InputList) - 37),
	((SELECT max(InputID) FROM InputList) - 36),
	((SELECT max(InputID) FROM InputList) - 35),
	((SELECT max(InputID) FROM InputList) - 34),
	((SELECT max(InputID) FROM InputList) - 33),
	((SELECT max(InputID) FROM InputList) - 32),
	((SELECT max(InputID) FROM InputList) - 31),
	((SELECT max(InputID) FROM InputList) - 30),
	((SELECT max(InputID) FROM InputList) - 29),
	((SELECT max(InputID) FROM InputList) - 28),
	((SELECT max(InputID) FROM InputList) - 27),
	((SELECT max(InputID) FROM InputList) - 26),
	((SELECT max(InputID) FROM InputList) - 25),
	((SELECT max(InputID) FROM InputList) - 24),
	((SELECT max(InputID) FROM InputList) - 23),
	((SELECT max(InputID) FROM InputList) - 22),
	((SELECT max(InputID) FROM InputList) - 21),
	((SELECT max(InputID) FROM InputList) - 20),
	((SELECT max(InputID) FROM InputList) - 19),
	((SELECT max(InputID) FROM InputList) - 18),
	((SELECT max(InputID) FROM InputList) - 17),
	((SELECT max(InputID) FROM InputList) - 16),
	((SELECT max(InputID) FROM InputList) - 15),
	((SELECT max(InputID) FROM InputList) - 14),
	((SELECT max(InputID) FROM InputList) - 13),
	((SELECT max(InputID) FROM InputList) - 12),
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

-- Player 0 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 268, 7 ); --KEY_STAR = KP_MULTIPLY
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 267, 7 ); --KEY_POUND = KP_DIVIDE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 256, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 257, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 258, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 259, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 260, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 261, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 262, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 263, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 264, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 265, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --StickLeft = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --StickRight = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --StickUp = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --StickDown = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --Paddle0Counter = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --Paddle0Clockwise = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --Paddle1Counter = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --Paddle1Clockwise = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 271, 7 ); --Bottom Button = KP_ENTER

INSERT INTO Player VALUES ( NULL, 1, 20, 100, 0, 
	(SELECT max(InputID) FROM InputList) - 8,
	(SELECT max(InputID) FROM InputList) - 7,
	(SELECT max(InputID) FROM InputList) - 6,
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	NULL,
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 1 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 113, 7 ); --KEY_STAR = q
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 9, 7 ); --KEY_POUND = TAB
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 48, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 49, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 50, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 51, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 52, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 53, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 54, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 55, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 56, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 57, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --StickLeft = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --StickRight = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --StickUp = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --StickDown = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --Paddle0Counter = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --Paddle0Clockwise = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --Paddle1Counter = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --Paddle1Clockwise = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 308, 7 ); --Bottom Button = LCTRL

INSERT INTO Player VALUES ( NULL, 1, 20, 100, 0, 
	(SELECT max(InputID) FROM InputList) - 8,
	(SELECT max(InputID) FROM InputList) - 7,
	(SELECT max(InputID) FROM InputList) - 6,
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
    NULL, 
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 2 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 268, 7 ); --KEY_STAR = KP_MULTIPLY
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 267, 7 ); --KEY_POUND = KP_DIVIDE
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 256, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 257, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 258, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 259, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 260, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 261, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 262, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 263, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 264, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 265, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --StickLeft = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --StickRight = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --StickUp = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --StickDown = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 276, 7 ); --Paddle0Counter = LEFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 275, 7 ); --Paddle0Clockwise = RIGHT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 273, 7 ); --Paddle1Counter = UP
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 274, 7 ); --Paddle1Clockwise = DOWN
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 271, 7 ); --Bottom Button = KP_ENTER

INSERT INTO Player VALUES ( NULL, 0, 20, 100, 0, 
	(SELECT max(InputID) FROM InputList) - 8,
	(SELECT max(InputID) FROM InputList) - 7,
	(SELECT max(InputID) FROM InputList) - 6,
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
	NULL,
	(SELECT max(KeypadID) FROM Keypad)
);

-- Player 3 --
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 113, 7 ); --KEY_STAR = q
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 9, 7 ); --KEY_POUND = TAB
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 48, 7 ); --KEY_0 = KP0
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 49, 7 ); --KEY_1 = KP1
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 50, 7 ); --KEY_2 = KP2
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 51, 7 ); --KEY_3 = KP3
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 52, 7 ); --KEY_4 = KP4
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 53, 7 ); --KEY_5 = KP5
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 54, 7 ); --KEY_6 = KP6
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 55, 7 ); --KEY_7 = KP7
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 56, 7 ); --KEY_8 = KP8
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 57, 7 ); --KEY_9 = KP9

INSERT INTO Keypad VALUES ( 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
	((SELECT max(InputID) FROM InputList) - 11),
	((SELECT max(InputID) FROM InputList) - 10),
	((SELECT max(InputID) FROM InputList) - 9),
	((SELECT max(InputID) FROM InputList) - 8),
	((SELECT max(InputID) FROM InputList) - 7),
	((SELECT max(InputID) FROM InputList) - 6),
	((SELECT max(InputID) FROM InputList) - 5),
	((SELECT max(InputID) FROM InputList) - 4),
	((SELECT max(InputID) FROM InputList) - 3),
	((SELECT max(InputID) FROM InputList) - 2),
	((SELECT max(InputID) FROM InputList) - 1),
	(SELECT max(InputID) FROM InputList)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --StickLeft = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --StickRight = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --StickUp = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --StickDown = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 97, 7 ); --Paddle0Counter = a
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 100, 7 ); --Paddle0Clockwise = d
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 119, 7 ); --Paddle1Counter = w
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 120, 7 ); --Paddle1Clockwise = x
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 308, 7 ); --Bottom Button = LCTRL

INSERT INTO Player VALUES ( NULL, 0, 20, 100, 0, 
	(SELECT max(InputID) FROM InputList) - 8,
	(SELECT max(InputID) FROM InputList) - 7,
	(SELECT max(InputID) FROM InputList) - 6,
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList),
    NULL, 
	(SELECT max(KeypadID) FROM Keypad)
);

INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 291, 7 ); --START = F10
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 292, 7 ); --SELECT = F11
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 293, 7 ); --OPTION = F12
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 306, 7 ); --CTRL = LCTRL
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 304, 7 ); --SHIFT = LSHIFT
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 318, 7 ); --BREAK = BREAK
INSERT INTO InputList VALUES ( NULL, 0, 0, 4, 289, 7 ); --RESET = F8

INSERT INTO Input VALUES ( 
    "Default 800 Settings", "", 0, 1, 
	(SELECT max(PlayerID) FROM Player) - 3,
	(SELECT max(PlayerID) FROM Player) - 2,
	(SELECT max(PlayerID) FROM Player) - 1,
	(SELECT max(PlayerID) FROM Player),
	(SELECT max(KeyboardID) FROM Keyboard),
	(SELECT max(InputID) FROM InputList) - 6,
	(SELECT max(InputID) FROM InputList) - 5,
	(SELECT max(InputID) FROM InputList) - 4,
	(SELECT max(InputID) FROM InputList) - 3,
	(SELECT max(InputID) FROM InputList) - 2,
	(SELECT max(InputID) FROM InputList) - 1,
	(SELECT max(InputID) FROM InputList)
);
END;

