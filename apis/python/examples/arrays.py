import asyncio as asio
import sys
sys.path.append('../')
from pprint import pprint

from ndb.client import NdbClient
from ndb.arrays import IntArrays, SortedIntArrays, ObjArrays, SortedStrArrays


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


async def iarr_create_sorted_unsorted():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  # create API object for unsorted and sorted int array
  unsortedArrays = IntArrays(client)
  sortedArrays = SortedIntArrays(client)

  # can use the same name because arrays are different type
  await unsortedArrays.create('my_array', 4)
  await sortedArrays.create('my_array', 4)

  await unsortedArrays.set_rng('my_array', 0, [100,50,200,10])
  await sortedArrays.set_rng('my_array', [100,50,200,10])

  # omit 'stop', get all values
  unsortedValues = await unsortedArrays.get_rng('my_array', start=0)
  sortedValues = await sortedArrays.get_rng('my_array', start=0)

  print(unsortedValues)
  print(sortedValues)


async def arr_set_multiple_types():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  objectArrays = ObjArrays(client)
  sortedStrArrays = SortedStrArrays(client)
  sortedIntArrays = SortedIntArrays(client)

  await objectArrays.create('student_records', 3)
  await sortedStrArrays.create('student_names_sorted', 3)
  await sortedIntArrays.create('student_scores', 3)
  
  # student records as json objects
  await objectArrays.set('student_records', 0, {'name':'Alice', 'modules':['Geography', 'Biology']})
  await objectArrays.set('student_records', 1, {'name':'Bob', 'modules':['Art', 'Philosophy']})
  await objectArrays.set('student_records', 2, {'name':'Charles', 'modules':['Music']})

  # sorted names
  await sortedStrArrays.set('student_names_sorted', 'Charles')
  await sortedStrArrays.set('student_names_sorted', 'Bob')
  await sortedStrArrays.set('student_names_sorted', 'Alice')

  # sorted leaderboard scores
  await sortedIntArrays.set('student_scores', 21)
  await sortedIntArrays.set('student_scores', 20)
  await sortedIntArrays.set('student_scores', 18)

  records = await objectArrays.get_rng('student_records', start=0)
  names = await sortedStrArrays.get_rng('student_names_sorted', start=0)
  scores = await sortedIntArrays.get_rng('student_scores', start=0)

  print('Records')
  pprint(records)
  print('Names')
  print(names)
  print('Scores')
  print(scores)


if __name__ == "__main__":
  for f in [iarr_unsorted_set_rng(), iarr_sorted_set_rng(),
            iarr_create_sorted_unsorted(), arr_set_multiple_types()]:
    
    print(f'---- {f.__name__} ----')
    asio.run(f)