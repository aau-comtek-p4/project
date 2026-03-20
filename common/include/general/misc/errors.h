#ifndef GENERAL_ERRORS_H
#define GENERAL_ERRORS_H

const char *custom_strerror(int err);
class ConnectionError {
public:
  enum {
    CONNECTION_FAILED = 1000,
    CONNECTION_LOST = 1001,
    CONNECTION_REFUSED = 1002,
    NOT_CONNECTED = 1003,
    ALREADY_CONNECTED = 1004,
    CONNECTION_TIMEOUT = 1005,
    HANDSHAKE_TIMEOUT = 1006,
  };
};

class SendError {
public:
  enum {
    SEND_FAILED = 2000,
    TX_BUFFER_FULL = 2001,
    TX_BUSY = 2002,
    TX_OVERFLOW = 2003,
    TRANSMISSION_ABORTED = 2004,
    SEND_TIMEOUT = 2005,
  };
};

class ReceiveError {
public:
  enum {
    RECEIVED_FAILED = 3000,
    RX_BUFFER_OVERFLOW = 3001,
    RX_BUFFER_EMPTY = 3002,
    PARTIAL_RECEIVE = 3003,
    CHECKSUM_ERROR = 3004,
    CRC_ERROR = 3005,
    CORRUPTED_PACKET = 3006,
    INVALID_PACKET = 3007,
    UNEXPECTED_PACKET = 3008,
    INVALID_HEADER = 3009,
    UNSUPPORTED_MESSAGE = 3010,
    RECEIVE_TIMEOUT = 3011,
  };
};

class ConfigurationError {
public:
  enum {
    INVALID_ADDRESS = 4000,
    INVALID_CHANNEL = 4001,
  };
};

class StateError {
public:
  enum {
    INVALID_STATE = 5000,
    OPERATION_NOT_ALLOWED = 5001,
    ALREADY_IN_PROGRESS = 5002,
    NOT_INITIALIZED = 5003,
  };
};

class AvailabilityError {
public:
  enum {
    RESOURCE_UNAVAILABLE = 6000,
    PERMISSION_DENIED = 6001,
    ALREADY_OPEN = 6002,
    NOT_OPEN = 6003,
    DEVICE_NOT_FOUND = 6004,
    DEVICE_BUSY = 6005,
    UNSUPPORTED_OPERATION = 6006,
  };
};

class OpenError {
public:
  enum {
    OPEN_FAILED = 7000,
    OPEN_TIMEOUT = 7001,
    INIT_FAILED = 7002,
  };
};

class ReadError {
public:
  enum {
    READ_FAILED = 8000,
    READ_TIMEOUT = 8001,
    END_OF_FILE = 8002,
    PARTIAL_READ = 8003,
  };
};

class WriteError {
public:
  enum {
    WRITE_FAILED = 9000,
    WRITE_TIMEOUT = 9001,
    PARTIAL_WRITE = 9002,
  };
};

class IntegrityError {
public:
  enum {
    INVALID_DATA = 10000,
    FORMAT_ERROR = 10001,
    DATA_CORRUPTED = 10002,
  };
};

class CapacityError {
public:
  enum {
    BUFFER_OVERFLOW = 11000,
    BUFFER_UNDERFLOW = 11001,
    OUT_OF_MEMORY = 11002,
    INSUFFICIENT_SPACE = 11003,
    OUTSIDE_RANGE = 11004,

  };
};

class TimeoutError {
public:
  enum {
    TIMEOUT = 12000,
    OPERATION_TIMEOUT = 12001,
  };
};

class HardwareError {
public:
  enum {
    HARDWARE_FAILURE = 13000,
    BUS_ERROR = 13001,
    DRIVER_ERROR = 13002,
  };
};

class CustomErrors : public ConnectionError,
                     public SendError,
                     public ReceiveError,
                     public ConfigurationError,
                     public StateError,
                     public AvailabilityError,
                     public OpenError,
                     public ReadError,
                     public WriteError,
                     public IntegrityError,
                     public CapacityError,
                     public TimeoutError,
                     public HardwareError {};

#endif
