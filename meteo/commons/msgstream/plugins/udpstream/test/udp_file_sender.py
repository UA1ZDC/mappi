#!/usr/bin/python3
import sys, socket, argparse
from os import listdir, path
import uuid
import zlib #, binascii

def read_in_chunks(file_object, chunk_size=1024):
  """Функция чтения файла по кусочкам

  Args:
      file_object (IO): файловый дескриптор
      chunk_size (int, optional): размер кусочка. Defaults to 1024.

  Yields:
      (binary): Бинарный кусочек данныъх
  """    
  while True:
    data = file_object.read(chunk_size)
    if not data:
      break
    yield data
    
        
def sendfile(filepath, sock, udp_port, udp_host, sender_id, attempts, chunk_size, no_wrap):
  """Отправка файлов по кусочкам через установленое сокет-соединение

  Args:
      filepath   (str) : путь к файлу, который необходимо отправить
      sock       (obj) : установленное сокет-соединение у
      udp_port   (int) : порт сервера, на который отправляется файл
      udp_host   (int) : имя сервера или IP адресс
      sender_id  (str) : уникальный идентификатор отправителя
      attempts   (int) : количество попыток отправки сообщения
      chunk_size (int) : размер одного сообщения в байтах
  """
  
  # sock.sendto(bytes("MESSAGE", "utf-8"), (udp_host, udp_port))

  # айди файла 
  magic     = uuid.uuid4().hex[:4].encode('utf-8')
  file_size = path.getsize(filepath)
  file_name = path.basename(filepath).encode('utf-8')
  id_start  = b'\x01\x01\x01'
  id_end    = b'\x03\x03\x03'

  def wrap_chunk(chunk):
    id_sender_length = len(sender_id)
    chunk_size_send  = len(bytearray(chunk))
    chunk_shift      = i*chunk_size
    crc32            = zlib.crc32(chunk) # & 0xffffffff  # crc32 2346018012

    msg_size         = bytes(4)
    msg              = bytearray(id_start)
    msg.extend( bytearray(msg_size          )                                  )
    msg.append( id_sender_length                                               )
    msg.extend( bytes(sender_id.encode('utf-8')   )                            )
    msg.extend( bytearray(magic             )                                  )
    msg.extend( file_size.to_bytes(4,byteorder='little', signed=False)            )
    msg.append( len(file_name)                                                 )
    msg.extend( bytearray(file_name         )                                  )
    msg.extend( chunk_size_send.to_bytes(4,byteorder='little', signed=False)      )
    msg.extend( chunk_shift.to_bytes(4,byteorder='little', signed=False)          )
    msg.extend( bytearray(chunk             )                                  )
    msg.extend( crc32.to_bytes(4,byteorder='little', signed=False)                )
    msg.extend( bytearray(id_end            )                                  )
    # заполняем размер
    msg[3:7] = len(msg).to_bytes(4,byteorder='little', signed=False)
    return msg

  
  with open (filepath,'rb') as fileio:
    i=0
    for chunk in read_in_chunks(fileio,chunk_size=chunk_size):
      try:
        # делаем несколько попыток отправки
        msg = bytearray(chunk)
        if not no_wrap:
          msg = wrap_chunk(chunk)
      except:
        print("Cant process convert to byte operation")
      # print(i, chunk_shift, chunk_size_send)
      for _ in range(attempts):
        
        try:
          sock.sendto( bytes(msg), (udp_host, udp_port))
        except:
          print("Cant send UDP package to host")
      i+=1
  

def main(args):  
  parser = argparse.ArgumentParser(description='UDP file sender script')
  parser.add_argument('-i', action="store", dest="host", default="localhost", type=str)
  parser.add_argument('-a', action="store", dest="attempts", default=3, type=int)
  parser.add_argument('-p', action="store", dest="port", default=5005, type=int)
  parser.add_argument('-s', action="store", dest="senderid", default="sender_1", type=str)
  parser.add_argument('-d', action="store", dest="path", default="/home/meteo/tlg", type=str)
  parser.add_argument('-f', action="store", dest="size", default=1024, type=int)
  parser.add_argument('-n', action="store", dest="no_wrap", default=False, type=bool)

  args = parser.parse_args()
  
  UDP_HOST    = args.host
  UDP_PORT    = args.port
  SENDER_ID   = args.senderid
  SENDER_PATH = args.path
  ATTEMPTS    = args.attempts
  CHUNK_SIZE  = args.size
  NO_SPLIT    = args.no_wrap
  
  try:
    onlyfiles = [path.join(SENDER_PATH, f) for f in listdir(SENDER_PATH) if path.isfile(path.join(SENDER_PATH, f))]
  except:
    print("Cant read folder")
    exit()
  
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
  
  if len(onlyfiles) >0:
    for f in onlyfiles:
      sendfile( f, sock, UDP_PORT, UDP_HOST, SENDER_ID, ATTEMPTS, CHUNK_SIZE, NO_SPLIT )
      try:
        if os.remove(f):
          print(f)
      except: 
        print( "Cant delete file %s" %f )
      # exit()
  
if __name__ == "__main__":
  main(sys.argv[1:])
