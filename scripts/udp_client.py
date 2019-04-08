import socket
import sys

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('localhost', 12121)
#request_msg = '\x63\x00\x00\x00{"command_id":1,"satellite_id":555555,"station_id":999999,"payload":"{\\"current_version\\":11235}"}'
request_msg = '\x49\x00\x00\x00{"command_id":3,"satellite_id":555555,"station_id":999999,"payload":"HOLO"}'

try:
    # Send data
    print >>sys.stderr, 'sending "%s"' % request_msg
    sent = sock.sendto(request_msg, server_address)

    # Receive response
    print >>sys.stderr, 'waiting to receive'
    data, server = sock.recvfrom(4096)
    print >>sys.stderr, 'received "%s"' % data

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
