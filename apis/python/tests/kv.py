import sys
import random
import asyncio as asio

sys.path.append('../')  # This is probably incorrect
from ndb.kvclient import KvClient


# TODO split test() into separate functions

async def test():
  client = KvClient()
  await client.open('ws://127.0.0.1:1987/')


  max = 10

  # clear before test
  await client.clear()  


  # set
  for i in range(0,max):
    await client.set({f"username{i}":f"user{i}", f"password{i}":f"passy{i}"})

  for i in range(0,max):
    await client.get((f"username{i}",f"password{i}"))


  # get
  usernameKeys = tuple()
  passwordKeys = tuple()
  for i in range(0,max):
    usernameKeys = usernameKeys + (f'username{i}',)
    passwordKeys = passwordKeys + (f'password{i}',)

  keys = await client.get(usernameKeys)
  assert len(keys) == max
  
  keys = await client.get(passwordKeys)
  assert len(keys) == max


  # add - don't overwrite existing key
  await client.add({'username0':'should_not_update'})
  keys = await client.get(('username0',))
  assert keys['username0'] == 'user0'  # unchanged from set above

  # add - new key
  await client.add({f'username{max}':'abc'})
  keys = await client.get((f'username{max}',))
  assert keys[f'username{max}'] == 'abc'  # added

  
  # rmv - remove key stored with add
  await client.rmv((f'username{max}',))
  keys = await client.get((f'username{max}',))
  assert len(keys) == 0 # key not found because we removed it
  

  # count
  count = await client.count()
  assert count == max*2  # *2 because we add usernameN and passwordN for range(0,max)
  

  # contains - first two keys exist, third key does not
  keys = await client.contains(('username0','username3', f'username{max*2}'))
  # keys only contains keys that exist
  assert (len(keys) == 2 and
          f'username{max*2}' not in keys)

  
  # clear_set - clear existing keys then set new keys
  count = await client.clear_set({'k1':'v1','k2':'v2','k3':'v3','k4':'v4'})
  assert count == max*2  # clear_set() returns number of keys cleared
  
    
  # keys
  keys = await client.keys()
  assert len(keys) == 4 and keys == ['k1','k2','k3','k4']


  # clear the keys set with clear_set
  count = await client.clear()
  assert count == 4



async def save_load(nKeys: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = KvClient()
  await client.open('ws://127.0.0.1:1987/')

  # empty db to ensure no keys
  cnt = await client.clear()

  # store some keys
  for i in range(0,nKeys):
    await client.set({f"username{i}":f"user{i}"})

  await client.save(dataSetName)


  # clear everything then load
  cnt = await client.clear()
  assert cnt == nKeys

  cnt = await client.load(dataSetName)
  assert cnt == nKeys


if __name__ == "__main__":
  for f in [test(), save_load(1000)]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

