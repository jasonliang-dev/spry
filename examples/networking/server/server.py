# python port of main.lua

import re
import socket
import time

udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp.settimeout(0)
udp.bind(("", 4242))
print("listening on port 4242")

state = {}
connections = {}

def entity(ident, data, addr):
  x, y = re.search(r"(\S*) (.*)", data).groups()
  state[ident] = (x, y)

def ping(ident, data, addr):
  arr = [f"[{k}]={{x={v[0]},y={v[1]}}}" for k, v in state.items()]
  payload = "state {" + ",".join(arr) + "}"
  udp.sendto(payload.encode(), addr)

cmd = {
  "entity": entity,
  "ping": ping,
}

while True:
  while True:
    try:
      data, addr = udp.recvfrom(1024)

      ident, head, tail = re.search(r"(\S*) (\S*) (.*)", data.decode()).groups()
      connections[ident] = time.time()

      if head in cmd:
        cmd[head](ident, tail, addr)
    except socket.error: break

  # remove connections after 3 seconds
  now = time.time()
  for k in list(connections.keys()):
    if now - connections[k] >= 3:
      del connections[k]
      del state[k]

  time.sleep(0.01)
