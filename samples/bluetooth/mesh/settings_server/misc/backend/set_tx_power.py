import socket
import time

UDP_PORT = 10000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

identifier = bytearray([0xde, 0xad, 0xfa, 0xce])
identifier.reverse()
opcode = bytearray([0x05])
dummy_mac = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
is_broadcast = bytearray([0x01])
txp_value_zero = bytearray([0x00])
txp_value_neg_forty = bytearray([0x0f])
txp_value_four = bytearray([0x04])
target_mac = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
target_mac.reverse()

package_neg_forty = identifier + opcode + dummy_mac + is_broadcast + txp_value_neg_forty + target_mac
package_zero = identifier + opcode + dummy_mac + is_broadcast + txp_value_zero + target_mac
package_four = identifier + opcode + dummy_mac + is_broadcast + txp_value_four + target_mac

print("Sending TXP value: -40 dB")
sock.sendto(bytearray(package_neg_forty), ('255.255.255.255', UDP_PORT))
time.sleep(8)

print("Sending TXP value: 4 dB")
sock.sendto(bytearray(package_four), ('255.255.255.255', UDP_PORT))
time.sleep(8)

print("Sending TXP value: 0 dB")
sock.sendto(bytearray(package_zero), ('255.255.255.255', UDP_PORT))