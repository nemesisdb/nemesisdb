import sys
import random
import asyncio as asio

sys.path.append('../')
from ndb import Client


# TODO split test() into separate functions
async def test():
  client = Client()
  listen_task = await client.listen('ws://127.0.0.1:1987/')

  if listen_task.done():
    print("server con failed")
    return


  max = 10

  # clear before test
  await client.clear()  


  # set
  for i in range(0,max):
    ok = await client.set({f"username{i}":f"user{i}", f"password{i}":f"passy{i}"})
    assert ok

  for i in range(0,max):
    (ok, rsp) = await client.get((f"username{i}",f"password{i}"))
    assert ok


  # get
  usernameKeys = tuple()
  passwordKeys = tuple()
  for i in range(0,max):
    usernameKeys = usernameKeys + (f'username{i}',)
    passwordKeys = passwordKeys + (f'password{i}',)

  (ok, keys) = await client.get(usernameKeys)
  assert ok and len(keys) == max
  
  (ok, keys) = await client.get(passwordKeys)
  assert ok and len(keys) == max


  # add - don't overwrite existing key
  ok = await client.add({'username0':'should_not_update'})
  assert ok
  (ok, keys) = await client.get(('username0',))
  assert keys['username0'] == 'user0'  # unchanged from set above

  # add - new key
  ok = await client.add({f'username{max}':'abc'})
  assert ok
  (ok, keys) = await client.get((f'username{max}',))
  assert keys[f'username{max}'] == 'abc'  # added

  
  # rmv - remove key stored with add
  ok = await client.rmv((f'username{max}',))
  assert ok
  (ok, keys) = await client.get((f'username{max}',))
  assert len(keys) == 0 # key not found because we removed it
  

  # count
  (ok, count) = await client.count()
  assert ok and count == max*2  # *2 because we add usernameN and passwordN for range(0,max)
  

  # contains - first two keys exist, third key does not
  (ok, keys) = await client.contains(('username0','username3', f'username{max*2}'))
  # keys only contains keys that exist
  assert (ok and
          len(keys) == 2 and
          f'username{max*2}' not in keys)

  
  # clear_set - clear existing keys then set new keys
  (ok, count) = await client.clear_set({'k1':'v1','k2':'v2','k3':'v3','k4':'v4'})
  assert ok and count == max*2  # clear_set() returns number of keys cleared
  
    
  # keys
  (ok, keys) = await client.keys()
  assert ok and len(keys) == 4 and keys == ['k1','k2','k3','k4']


  # clear the keys set with clear_set
  (ok, count) = await client.clear()
  assert ok and count == 4

  
  await client.close()
  await listen_task


async def save_load(nKeys: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = Client()
  await client.listen('ws://127.0.0.1:1987/')

  # empty db to ensure no keys
  (cleared, cnt) = await client.clear()
  assert cleared


  # store some keys
  for i in range(0,nKeys):
    ok = await client.set({f"username{i}":f"user{i}"})
    assert ok

  saved = await client.save(dataSetName)
  assert saved


  # clear everything then load
  (cleared, cnt) = await client.clear()
  assert cleared and cnt == nKeys

  (loaded, cnt) = await client.load(dataSetName)
  assert loaded and cnt == nKeys


if __name__ == "__main__":
  for f in [test(), save_load(1000)]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

