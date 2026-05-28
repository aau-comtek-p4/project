#include "general/misc/errors.h"
#include <cstring>

const char *custom_strerror(ErrorWrapper err) {
  if (err.tag == ErrorWrapper::ERRNO) {
    return strerror(err.error);
  }
  switch (err.error) {
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
  case CustomErrors::FAILED_SETUP:
    return "Setup failed";

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
  case CustomErrors::MISSED_TICK:
    return "Clock missed tick";

  default:
    return "Unknown error";
  }
}
