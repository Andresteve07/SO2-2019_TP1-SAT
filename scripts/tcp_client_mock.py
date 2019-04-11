import socket                   # Import socket module
import sys
import json
import time

s = socket.socket()             # Create a socket object
host = "localhost"  #Ip address that the TCPServer  is there
port = 12121                     # Reserve a port for your service every new transfer wants a new port or you must wait.

s.connect((host, port))
#s.send("Hello server!")
request_msg = '\x49\x00\x00\x00{"command_id":2,"satellite_id":555555,"station_id":999999,"payload":"HOLO"}'

def recv_file(socketinst, filename, file_size):
    recv_bytes = 0
    with open(filename, 'wb') as f:
        print ('file opened %s',filename)
        while True:
            #print('receiving data...')
            data = socketinst.recv(32)
            #print('recved=%i data=%s',len(data), (data))
            recv_bytes+=len(data)
            #print('bytesread: %i -- partial count %i',len(data), recv_bytes)
            if recv_bytes >= file_size:
                break
            # write data to a file
            f.write(data)
        print("FINAL COUNT: %i",recv_bytes)
        f.close()
def get_payload_size(buffer_header):
    payload_size = 0
    payload_size = buffer_header[3]
    payload_size = (payload_size*256) + buffer_header[2]
    payload_size = (payload_size*256) + buffer_header[1]
    payload_size = (payload_size*256) + buffer_header[0]
    print("PAYLOAD SIZE: ",payload_size)
    return payload_size

def image_scan(socketinst):
    request_msg = '\x56\x00\x00\x00{"command_id":2,"satellite_id":555555,"station_id":999999,"payload":null,"error":null}'
    # Send data
    print >>sys.stderr, 'sending "%s"' % request_msg
    s.send(request_msg)
    # Receive response
    print >>sys.stderr, 'waiting to receive'
    data, server = s.recvfrom(504)
    print >>sys.stderr, 'received "%s"' % data
    #get_payload_size(data)
    #print 'SUBSET "%s"' % data[5:]
    string_data = data[4:].decode('ASCII')
    parsed_json = json.loads(string_data)
    print(parsed_json['command_id'])
    response_string = parsed_json['payload']
    print(response_string)
    parsed_payload = json.loads(response_string)
    print(parsed_payload['slice_quantity'])
    slices_meta = parsed_payload['slices_dataset']
    for sliced in slices_meta:
        print('Receiving FILE: %s OF SIZE: %i',sliced['name'],sliced['size'])
        recv_file(socketinst,sliced['name'],sliced['size'])
        #time.sleep(15)

def send_file(socketinst):
    filename='test_update_file' #In the same folder or path is this file running must the file you want to tranfser to be
    #$ echo -n -e '\xff' >> file_server #This adds an EOF to the end of the file.
    #$ dd if=/dev/urandom of=file_server count=32 bs=1024
    f = open(filename,'rb')
    l = f.read(512)
    while (l):
        socketinst.send(l)
        l = f.read(512)
    print('Sent ',filename)
    f.close()

def firmware_update(socketinst):
    request_msg = '\x8B\x00\x00\x00{"command_id":1,"satellite_id":555555,"station_id":999999,"error":null,"payload":"{\\"update_version\\":10502,\\"file_size_bytes\\":1024}"}'
    # Send data
    print >>sys.stderr, 'sending "%s"' % request_msg
    s.send(request_msg)
    # Receive response
    print >>sys.stderr, 'waiting to receive'
    data, server = s.recvfrom(504)
    print >>sys.stderr, 'received "%s"' % data
    time.sleep(2)
    send_file(socketinst)
try:
    #image_scan(s)
    firmware_update(s)

finally:
    print >>sys.stderr, 'closing socket'
    s.close()