import asyncio as asio
import sys
sys.path.append('../')

from ndb.client import NdbClient
from ndb.arrays import IntArrays, SortedIntArrays


async def iarr_unsorted_set_rng():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  # create API object
  arrays = IntArrays(client)
  
  # create array called 'example' of length 4
  await arrays.create('example', 4)
  
  # set four integers
  await arrays.set_rng('example', 0, [100,50,200,10])
  
  # get all values: stop is exclusive
  allValues = await arrays.get_rng('example', start=0, stop=5)
  print(allValues)


async def iarr_sorted_set_rng():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  sortedArrays = SortedIntArrays(client)

  await sortedArrays.create('sorted', 4)

  # set four integers
  await sortedArrays.set_rng('sorted', [100,50,200,10])
  
  # get all values: stop is exclusive
  allValues = await sortedArrays.get_rng('sorted', start=0, stop=5)
  print(allValues)



if __name__ == "__main__":
  for f in [iarr_unsorted_set_rng(), iarr_sorted_set_rng()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)