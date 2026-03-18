#ifndef GENERAL_ERRORS_H
#define GENERAL_ERRORS_H

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
const char *custom_strerror(int err) {
  switch (err) {
  // ConnectionError
  case CustomErrors::CONNECTION_FAILED:
    return "Connection failed";
  case CustomErrors::CONNECTION_LOST:
    return "Connection lost";
  case CustomErrors::CONNECTION_REFUSED:
    return "Connection refused";
  case CustomErrors::NOT_CONNECTED:
    return "Not connected";
  case CustomErrors::ALREADY_CONNECTED:
    return "Already connected";
  case CustomErrors::CONNECTION_TIMEOUT:
    return "Connection timed out";
  case CustomErrors::HANDSHAKE_TIMEOUT:
    return "Handshake timed out";

  // SendError
  case CustomErrors::SEND_FAILED:
    return "Send failed";
  case CustomErrors::TX_BUFFER_FULL:
    return "Transmit buffer full";
  case CustomErrors::TX_BUSY:
    return "Transmitter busy";
  case CustomErrors::TX_OVERFLOW:
    return "Transmit overflow";
  case CustomErrors::TRANSMISSION_ABORTED:
    return "Transmission aborted";
  case CustomErrors::SEND_TIMEOUT:
    return "Send timed out";

  // ReceiveError
  case CustomErrors::RECEIVED_FAILED:
    return "Receive failed";
  case CustomErrors::RX_BUFFER_OVERFLOW:
    return "Receive buffer overflow";
  case CustomErrors::RX_BUFFER_EMPTY:
    return "Receive buffer empty";
  case CustomErrors::PARTIAL_RECEIVE:
    return "Partial receive";
  case CustomErrors::CHECKSUM_ERROR:
    return "Checksum error";
  case CustomErrors::CRC_ERROR:
    return "CRC error";
  case CustomErrors::CORRUPTED_PACKET:
    return "Corrupted packet";
  case CustomErrors::INVALID_PACKET:
    return "Invalid packet";
  case CustomErrors::UNEXPECTED_PACKET:
    return "Unexpected packet";
  case CustomErrors::INVALID_HEADER:
    return "Invalid header";
  case CustomErrors::UNSUPPORTED_MESSAGE:
    return "Unsupported message";
  case CustomErrors::RECEIVE_TIMEOUT:
    return "Receive timed out";

  // ConfigurationError
  case CustomErrors::INVALID_ADDRESS:
    return "Invalid address";
  case CustomErrors::INVALID_CHANNEL:
    return "Invalid channel";

  // StateError
  case CustomErrors::INVALID_STATE:
    return "Invalid state";
  case CustomErrors::OPERATION_NOT_ALLOWED:
    return "Operation not allowed";
  case CustomErrors::ALREADY_IN_PROGRESS:
    return "Already in progress";
  case CustomErrors::NOT_INITIALIZED:
    return "Not initialized";

  // AvailabilityError
  case CustomErrors::RESOURCE_UNAVAILABLE:
    return "Resource unavailable";
  case CustomErrors::PERMISSION_DENIED:
    return "Permission denied";
  case CustomErrors::ALREADY_OPEN:
    return "Already open";
  case CustomErrors::NOT_OPEN:
    return "Not open";
  case CustomErrors::DEVICE_NOT_FOUND:
    return "Device not found";
  case CustomErrors::DEVICE_BUSY:
    return "Device busy";
  case CustomErrors::UNSUPPORTED_OPERATION:
    return "Unsupported operation";

  // OpenError
  case CustomErrors::OPEN_FAILED:
    return "Open failed";
  case CustomErrors::OPEN_TIMEOUT:
    return "Open timed out";
  case CustomErrors::INIT_FAILED:
    return "Initialization failed";

  // ReadError
  case CustomErrors::READ_FAILED:
    return "Read failed";
  case CustomErrors::READ_TIMEOUT:
    return "Read timed out";
  case CustomErrors::END_OF_FILE:
    return "End of file";
  case CustomErrors::PARTIAL_READ:
    return "Partial read";

  // WriteError
  case CustomErrors::WRITE_FAILED:
    return "Write failed";
  case CustomErrors::WRITE_TIMEOUT:
    return "Write timed out";
  case CustomErrors::PARTIAL_WRITE:
    return "Partial write";

  // IntegrityError
  case CustomErrors::INVALID_DATA:
    return "Invalid data";
  case CustomErrors::FORMAT_ERROR:
    return "Format error";
  case CustomErrors::DATA_CORRUPTED:
    return "Data corrupted";

  // CapacityError
  case CustomErrors::BUFFER_OVERFLOW:
    return "Buffer overflow";
  case CustomErrors::BUFFER_UNDERFLOW:
    return "Buffer underflow";
  case CustomErrors::OUT_OF_MEMORY:
    return "Out of memory";
  case CustomErrors::INSUFFICIENT_SPACE:
    return "Insufficient space";
  case CustomErrors::OUTSIDE_RANGE:
    return "Outside range";

  // TimeoutError
  case CustomErrors::TIMEOUT:
    return "Timeout";
  case CustomErrors::OPERATION_TIMEOUT:
    return "Operation timed out";

  // HardwareError
  case CustomErrors::HARDWARE_FAILURE:
    return "Hardware failure";
  case CustomErrors::BUS_ERROR:
    return "Bus error";
  case CustomErrors::DRIVER_ERROR:
    return "Driver error";

  default:
    return "Unknown error";
  }
}
#endif
